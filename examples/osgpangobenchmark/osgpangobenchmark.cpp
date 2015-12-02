// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgPango/TextTransform>
#include <osgPango/ShaderManager>

#include "ConvertPangoText.h"

#if 0
typedef osg::Vec3dArray LabelPath;
#else
class LabelPath : public osg::Vec3dArray
{
public:
    LabelPath()
    {
    }

    osg::Vec3d computePosition(double d, bool fromEnd)
    {
        if (empty()) return osg::Vec3d();

        if (size()==1) return front();

        int delta = fromEnd ? -1 : 1;

        osg::Vec3d* first = &at(0);
        osg::Vec3d* last = &at(size()-1);

        osg::Vec3d* currVertex = fromEnd ? last : first;
        if (d==0.0) return *currVertex;

        osg::Vec3d* lastVertex = fromEnd ? first : last;
        osg::Vec3d* nextVertex = currVertex + delta;

        double len = ((*nextVertex)-(*currVertex)).length();
        double currDistance = 0.0;
        double nextDistance = len;

        while(d>nextDistance && nextVertex!=lastVertex)
        {
            currDistance = nextDistance;
            currVertex = nextVertex;
            nextVertex += delta;

            len = ((*nextVertex)-(*currVertex)).length();
            nextDistance = currDistance + len;
        }

        if (d>nextDistance)
        {
            // OSG_NOTICE<<"computePosition("<<d<<") past the end"<<std::endl;
            return *lastVertex;
        }

        double r = (d-currDistance) / (nextDistance-currDistance);
        osg::Vec3d v = (*currVertex)*(1.0-r)+(*nextVertex)*r;

        return v;
    }
};
#endif


class MapQuadsToPath : public osg::NodeVisitor
{
public:

    MapQuadsToPath(LabelPath* labelPath, const osg::Vec3d& normal, double scale, bool flipPath):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _labelPath(labelPath),
        _normal(normal),
        _scale(scale),
        _flipPath(flipPath)
    {
    }

    void processQuad(osg::Vec3& v1, osg::Vec3& v2, osg::Vec3& v3,osg::Vec3& v4)
    {
        // find positions along labelPath
        double min_x = v1.x() * _scale;
        double max_x = v2.x() * _scale;

        osg::Vec3d p_min = _labelPath->computePosition(min_x, _flipPath);
        osg::Vec3d p_max = _labelPath->computePosition(max_x, _flipPath);

//        OSG_NOTICE<<"         processQuad( ("<<v1<<"), ("<<v2<<"), ("<<v3<<"), ("<<v4<<") ) min_x="<<min_x<<", max_x="<<max_x<<std::endl;
//        OSG_NOTICE<<"             p_min=("<<p_min<<") p_max=("<<p_max<<")"<<std::endl;

        osg::Vec3 dx = (p_max-p_min);
        dx.normalize();

        osg::Vec3d dy = _normal^dx;
        dy.normalize();

        v1 = osg::Vec3(p_min) + dy*(v1.y()*_scale);
        v2 = osg::Vec3(p_max) + dy*(v2.y()*_scale);
        v3 = osg::Vec3(p_max) + dy*(v3.y()*_scale);
        v4 = osg::Vec3(p_min) + dy*(v4.y()*_scale);
    }

    void apply(osg::Geometry& geometry)
    {
        osg::Vec3Array* vertices_float = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
        if (!vertices_float) return;

        osg::Geometry::PrimitiveSetList& primitives = geometry.getPrimitiveSetList();
        for(osg::Geometry::PrimitiveSetList::iterator itr = primitives.begin();
            itr != primitives.end();
            ++itr)
        {
            osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(itr->get());
            if (da)
            {
                if (da->getMode()==GL_QUADS)
                {
                    int last_quad_start = da->getFirst()+da->getCount()-4;
                    for(int i=da->getFirst(); i<=last_quad_start; i+=4)
                    {
                        processQuad((*vertices_float)[i], (*vertices_float)[i+1], (*vertices_float)[i+2], (*vertices_float)[i+3]);
                    }
                }
            }
        }
    }

    osg::ref_ptr<LabelPath>     _labelPath;
    osg::Vec3d                  _normal;
    double                      _scale;
    bool                        _flipPath;
};


class ProcessRoads : public osg::NodeVisitor
{
public:
    ProcessRoads(const std::string& renderer, const osgPango::TextOptions& to, double labelHeight, double labelSpacing, const std::string& prefix, const std::string& postfix):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _numLabels(0),
        _renderer(renderer),
        _textOptions(to),
        _labelHeight(labelHeight),
        _labelSpacing(labelSpacing),
        _prefix(prefix),
        _postfix(postfix)
    {
        _labelSubgraph = new osg::Group;
    }

    void apply(osg::Node& node)
    {
        if(!node.getName().empty())
        {
            _nameStack.push_back(node.getName());
            traverse(node);
            _nameStack.pop_back();
        }
        else
        {
            traverse(node);
        }
    }

    void apply(osg::Geometry& geometry)
    {
        bool hasName = !geometry.getName().empty();
        if(hasName) _nameStack.push_back(geometry.getName());

        std::string inhertedName = geometry.getName();
        if(inhertedName.empty() && !_nameStack.empty()) inhertedName = _nameStack.back();

        osg::Array* vertices = geometry.getVertexArray();
        osg::Geometry::PrimitiveSetList& primitives = geometry.getPrimitiveSetList();
        for(osg::Geometry::PrimitiveSetList::iterator itr = primitives.begin();
            itr != primitives.end();
            ++itr)
        {
            osg::PrimitiveSet* prim = itr->get();
            const std::string& primName = prim->getName();
            const std::string& name = (!primName.empty()) ? primName : inhertedName;

            if (!name.empty()) createRoadLabel(name, vertices, prim);
        }


        if(hasName) _nameStack.pop_back();
    }

    template<class T>
    double computeLength(T* vertices, int first, int last)
    {
        if (first>=last) return 0.0;

        double length = 0.0;

        for(unsigned int i = first; i<last; ++i)
        {
            length += ((*vertices)[i+1]-(*vertices)[i]).length();
        }

        return length;
    }

    template<class T>
    int computeLabelPath(T* vertices, int& currIndex, int endIndex, double& currDistance, double pathStart, double pathEnd, osg::Vec3dArray& path, osg::NotifySeverity debugging)
    {
        if (debugging) { OSG_NOTICE<<"Debug mode computeLabelPath(vertices, currIndex="<<currIndex<<", endIndex="<<endIndex<<", currDistance="<<currDistance<<", pathStart="<<pathStart<<", pathEnd="<<pathEnd<<")"<<std::endl; }


        if (currIndex>=endIndex) return 0;

        osg::Vec3d current = (*vertices)[currIndex];
        osg::Vec3d next = (*vertices)[currIndex+1];
        osg::Vec3d dv = next-current;
        double segmentLength = dv.length();
        double nextDistance = currDistance+segmentLength;

        if (debugging) { OSG_NOTICE<<"    segmentLength= "<<segmentLength<<", currIndex="<<currIndex<<", currDistance="<<currDistance<<", nextDistance="<<nextDistance<<std::endl; }

        // find segment that contains the start position
        while(nextDistance<pathStart && (currIndex+1)<endIndex)
        {
            ++currIndex;

            currDistance = nextDistance;
            current = next;
            next = (*vertices)[currIndex+1];

            dv = next-current;
            segmentLength = dv.length();
            nextDistance = currDistance+segmentLength;

            if (debugging) { OSG_NOTICE<<"    segmentLength= "<<segmentLength<<", currIndex="<<currIndex<<", currDistance="<<currDistance<<", nextDistance="<<nextDistance<<std::endl; }
        }

        if (currIndex>=endIndex) return 1;

        // compute vertex position of pathStart along the current segment
        osg::Vec3d v = (segmentLength==0.0) ? current : current+dv*((pathStart-currDistance)/segmentLength);

        // add first vertex onto path.
        path.push_back(v);

        if (debugging) { OSG_NOTICE<<"    first vertex: path.push_back("<<v<<")"<<std::endl;}

        // step along the line till the next segment exceeds the pathEnd or we run out of line
        while(nextDistance<pathEnd && (currIndex+1)<endIndex)
        {
            ++currIndex;

            currDistance = nextDistance;
            current = next;
            next = (*vertices)[currIndex+1];

            path.push_back(current);

            if (debugging) { OSG_NOTICE<<"    mid vertex: path.push_back("<<current<<")"<<std::endl;}

            dv = next-current;
            segmentLength = dv.length();
            nextDistance = currDistance+segmentLength;

                if (debugging) { OSG_NOTICE<<"    segmentLength= "<<segmentLength<<", currIndex="<<currIndex<<", currDistance="<<currDistance<<", nextDistance="<<nextDistance<<std::endl; }
        }

        if (nextDistance<pathEnd)
        {
            if (debugging) { OSG_NOTICE<<"    early end vertex: path.push_back("<<current<<")"<<std::endl;}
            path.push_back(next);
            return 2;
        }

        // compute vertex position of pathStart along the current segment
        v = (segmentLength==0.0) ? current : current+dv*((pathEnd-currDistance)/segmentLength);
        // add first vertex onto path.
        path.push_back(v);

        if (debugging) { OSG_NOTICE<<"    end vertex: path.push_back("<<current<<")"<<std::endl;}

        return 3;
    }

    inline osgPango::TextTransform* createTextNoSubstitution(const std::string& name)
    {
        osgPango::TextTransform* t = new osgPango::TextTransform;
        t->setGlyphRenderer(_renderer);
        t->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_LEFT_CENTER);

        std::string completeText(_prefix + name + _postfix);
        t->setText(completeText);

        // OSG_NOTICE<<"setText("<<completeText<<")"<<std::endl;

        t->finalize();

        ++_numLabels;

        return t;
    }

    osgPango::TextTransform* createText(const std::string& name)
    {
        std::size_t amp_pos = name.find('&');
        if (amp_pos!=std::string::npos)
        {
            // OSG_NOTICE<<"substituing name ["<<name<<"]"<<std::endl;

            std::string newString = name;
            std::string ampString("&amp;");

            while(amp_pos!=std::string::npos)
            {
                newString.replace(amp_pos, 1, ampString);
                amp_pos = name.find('&', amp_pos+ampString.size());
            }

            return createTextNoSubstitution(newString);
        }
        else
        {
            return createTextNoSubstitution(name);
        }
    }

    void scaleAndPositionText(osgPango::TextTransform* text, const osg::Vec3d& position)
    {
        //OSG_NOTICE<<"scaleAndPositionText("<<text<<") position = "<<position<<" size="<<text->getSize()<<std::endl;
        osg::Vec2 size = text->getSize();
        double scale = _labelHeight/size.y();
        text->setMatrix(osg::Matrix::scale(scale, scale, scale) * osg::Matrix::translate(position));
    }


    void placeTextEndPoints(osgPango::TextTransform* text, const LabelPath* path)
    {
        if (path->empty()) return;

        if (path->size()==1)
        {
            scaleAndPositionText(text, path->front());
            _labelSubgraph->addChild(text);
            return;
        }

        osg::Vec2 size = text->getSize();
        double scale = _labelHeight/size.y();

        osg::Vec3d start = path->front();
        osg::Vec3d end = path->back();

        osg::Vec3d dx = (end-start); // vector along road
        dx.normalize();

        osg::Vec3d dz = (start+end); // up vector
        dz.normalize();

        osg::Vec3d dy = (dz^dx);
        dy.normalize();

        osg::Vec3d lv(0.0,1.0,0.0);

        if (lv*dy>0.0)
        {
            osg::Matrixd matrix;
            matrix.set(dx.x(), dx.y(), dx.z(), 0.0,
                    dy.x(), dy.y(), dy.z(), 0.0,
                    dz.x(), dz.y(), dz.z(), 0.0,
                    start.x(), start.y(), start.z(), 1.0);

            text->setMatrix(osg::Matrix::scale(scale, scale, scale) * matrix);
        }
        else
        {
            osg::Matrixd matrix;
            matrix.set(-dx.x(), -dx.y(), -dx.z(), 0.0,
                    -dy.x(), -dy.y(), -dy.z(), 0.0,
                    -dz.x(), -dz.y(), -dz.z(), 0.0,
                    end.x(), end.y(), end.z(), 1.0);

            text->setMatrix(osg::Matrix::scale(scale, scale, scale) * matrix);
        }

        _labelSubgraph->addChild(text);
    }

    void placeTextFollowPath(osgPango::TextTransform* text, LabelPath* path)
    {
        osg::Vec2 size = text->getSize();
        double scale = _labelHeight/size.y();

        osg::Vec3d start = path->front();
        osg::Vec3d end = path->back();

        osg::Vec3d dx = (end-start); // vector along road
        dx.normalize();

        osg::Vec3d dz = (start+end); // up vector
        dz.normalize();

        osg::Vec3d dy = (dz^dx);
        dy.normalize();

        osg::Vec3d lv(0.0,1.0,0.0);

        bool swapEnds = (lv*dy<0.0);

        MapQuadsToPath mapQuadsToPath(path, dz, scale, swapEnds);

        text->accept(mapQuadsToPath);

        _labelSubgraph->addChild(text);
    }

    void placeText(osgPango::TextTransform* text, LabelPath* path)
    {
        bool followPath = true;
        if (followPath) placeTextFollowPath(text, path);
        else placeTextEndPoints(text, path);
    }


    void createRoadLabel(const std::string& name, osg::Array* vertices, osg::PrimitiveSet* prim)
    {
        osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(prim);
        osg::Vec3Array* vetices_float = dynamic_cast<osg::Vec3Array*>(vertices);
        osg::Vec3dArray* vetices_double = dynamic_cast<osg::Vec3dArray*>(vertices);
        bool use_float = vetices_float!=0;

        if (da)
        {
            int first = da->getFirst();
            int last = first + da->getCount()-1;
            osg::Vec3d first_vertex = use_float ? osg::Vec3d((*vetices_float)[first]) : (*vetices_double)[first];
            double roadLength = use_float ? computeLength(vetices_float, first, last) : computeLength(vetices_double, first, last);


            osg::ref_ptr<osgPango::TextTransform> text = createText(name);

            osg::Vec2 text_size = text->getSize();
            double textScale = _labelHeight/text_size.y();
            double textLength = text_size.x()*textScale;


            if (textLength<roadLength)
            {
                double e = 0.0;
                double numberLabels = floor((roadLength - 2.0*e + _labelSpacing) / (textLength+_labelSpacing));
                e = (roadLength - numberLabels*textLength - _labelSpacing*(numberLabels-1.0))/2.0;

                //OSG_NOTICE<<"createRoadLabel("<<name<<", "<<first_vertex<<") road length = "<<roadLength<<", text length ="<<textLength<<std::endl;
                //OSG_NOTICE<<"    numberLabels = "<<numberLabels<<", e = "<<e<<std::endl;

                osg::ref_ptr<LabelPath> path = new LabelPath;
                int currIndex = da->getFirst();
                int endIndex = currIndex+da->getCount()-1;
                double currDistance = 0.0;
                double pathStart = e;
                double pathEnd = pathStart+textLength;
                for(unsigned int i=0; i<static_cast<unsigned int>(numberLabels); ++i)
                {
                    int prev_currIndex = currIndex;
                    double prev_currDistance = currDistance;

                    int result = use_float ?
                        computeLabelPath(vetices_float, currIndex, endIndex, currDistance, pathStart, pathEnd, *path, osg::ALWAYS) :
                        computeLabelPath(vetices_double, currIndex, endIndex, currDistance, pathStart, pathEnd, *path, osg::ALWAYS);

                    // debug checks
                    double newLabelLength = computeLength(path.get(), 0, int(path->size())-1);
                    if (!osg::equivalent(newLabelLength,textLength))
                    {
                        path->clear();

                        currIndex = prev_currIndex;
                        currDistance = prev_currDistance;

                        int result = use_float ?
                            computeLabelPath(vetices_float, currIndex, endIndex, currDistance, pathStart, pathEnd, *path, osg::DEBUG_INFO) :
                            computeLabelPath(vetices_double, currIndex, endIndex, currDistance, pathStart, pathEnd, *path, osg::DEBUG_INFO);

                        if (newLabelLength<textLength) { OSG_NOTICE<<"      Warning: Label segment length SHORTER than text label length"; }
                        else { OSG_NOTICE<<"      Warning: Label segment length LONGER than text label length"; }

                        OSG_NOTICE<<"          path.size()="<<path->size()<<" length="<<newLabelLength<<", result="<<result<<std::endl;
                        OSG_NOTICE<<"          prev_currIndex="<<prev_currIndex<<"\t prev_currDistance="<<prev_currDistance<<std::endl;
                        OSG_NOTICE<<"               currIndex="<<currIndex<<"\t      currDistance="<<currDistance<<std::endl;
                    }

                    if (i>0)
                    {
                        text = createText(name);
                    }

                    placeText(text, path);

                    // move pathStart/End to next label position
                    pathStart = pathEnd + _labelSpacing;
                    pathEnd = pathStart + textLength;
                    path->clear();
                }
#if 0
                scaleAndPositionText(text, first_vertex);

                _labelSubgraph->addChild(text);
#endif
            }
            else
            {
                //OSG_NOTICE<<"Road too short createRoadLabel("<<name<<", "<<first_vertex<<") road length = "<<roadLength<<", text length ="<<textLength<<std::endl;
            }

        }
    }

    osg::ref_ptr<osg::Node> getRoadLabels()
    {
        return _labelSubgraph;
    }

    typedef std::vector<std::string> NameStack;
    NameStack _nameStack;


    unsigned int                _numLabels;

    std::string                 _renderer;
    osgPango::TextOptions       _textOptions;
    double                      _labelHeight;
    double                      _labelSpacing;
    std::string                 _prefix;
    std::string                 _postfix;

    osg::ref_ptr<osg::Group>    _labelSubgraph;
};


int main(int argc, char** argv)
{
    osg::ArgumentParser args(&argc, argv);

    // initialize osgPango::Context which will be used to render the glyphs
    osgPango::Context& context = osgPango::Context::instance();
    context.init();

    if(args.read("--help"))
    {
        args.getApplicationUsage()->write( std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION );
        return 0;
    }

    osgPango::TextOptions to;
    to.width = 600;

    std::string renderer, rendererSize;
    while(args.read("--renderer", renderer, rendererSize))
    {
        int          arg1, arg2 = 0;
        unsigned int arg3, arg4 = 0;

        std::sscanf(rendererSize.c_str(), "%i,%i,%u,%u", &arg1, &arg2, &arg3, &arg4);

        osgPango::GlyphRenderer* r = 0;

        if(renderer == "outline")
            r = new osgPango::GlyphRendererOutline(arg1);
        else if(renderer == "shadow")
            r = new osgPango::GlyphRendererShadow(arg1, arg2);
        else if(renderer == "shadowBlur")
            r = new osgPango::GlyphRendererShadowBlur( arg1, arg2, arg3, arg4 );
        else if(renderer == "shadowInset")
            r = new osgPango::GlyphRendererShadowInset( arg1, arg2, arg3, arg4);
        else
        {
            OSG_NOTICE << "Bad renderer: " << renderer << std::endl;
            continue;
        }

        if(r) context.addGlyphRenderer(renderer, r);
    }




    osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::ref_ptr<osg::Node> root = group;

    bool assignedText = false;

    std::string filename;
    while (args.read("-m",filename))
    {
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(filename);
        if (node) group->addChild(node);
    }

    std::string prefix;
    while (args.read("--prefix", prefix)) {}

    std::string postfix;
    while (args.read("--postfix", postfix)) {}

    double labelHeight = 1.0;
    while (args.read("--label-height", labelHeight)) {}

    double labelSpacing = 100.0;
    while (args.read("--label-spacing", labelSpacing)) {}

    while (args.read("--road", filename))
    {
        osg::ElapsedTime elapsedTime;

        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(filename);

        if (node)
        {
            OSG_NOTICE<<"Reading of scene graph took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;

            elapsedTime.reset();

            ProcessRoads processRoads(renderer, to, labelHeight, labelSpacing, prefix, postfix);
            node->accept(processRoads);

            group->addChild(node);

            osg::ref_ptr<osg::Node> roadLabels = processRoads.getRoadLabels();
            if (roadLabels)
            {
                assignedText = false;
                group->addChild(roadLabels);

                //root = roadLabels;
            }

            OSG_NOTICE<<"ProcessRoad to create "<<processRoads._numLabels<<" road labels took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;
        }

    }

    if (!assignedText)
    {
        const std::string LOREM_IPSUM(
            "<span font='Verdana 20'>"
            "<span color='red'><b>Lorem ipsum dolor sit amet</b>, consectetur adipisicing elit, sed do eiusmod</span> "
            "<span color='orange'>tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,</span> "
            "<span font='Verdana 15'><i>"
            "<span color='yellow'>quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.</span> "
            "</i></span>"
            "<span color='green'>Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu</span> "
            "<span color='white'>fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in</span> "
            "<span color='black'>culpa qui officia deserunt mollit anim id est laborum.</span>"
            "</span>"
        );

        // default text
        std::string text = LOREM_IPSUM;

        while(args.read("-t",text)) {}

        osg::ref_ptr<osgPango::TextTransform> t = new osgPango::TextTransform();
        t->setGlyphRenderer(renderer);
        t->setAxisAlignment(osgPango::TextTransform::AXIS_ALIGN_XZ_PLANE);
        t->setText(text, to);

        if(!t->finalize()) return 1;

        group->addChild(t);
    }

    if (args.read("--convert"))
    {
        osg::ElapsedTime elapsedTime;

        ConvertPangoText convertPangoText;
        root->accept(convertPangoText);

        root = convertPangoText.getSubgraph();

        OSG_NOTICE<<"Conversion of scene graph took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;
    }

    if (args.read("--optimize"))
    {
        osg::ElapsedTime elapsedTime;

        osgUtil::Optimizer optimizer;
        optimizer.optimize(root);

        OSG_NOTICE<<"Optimization of scene graph took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;
    }

    std::string outputFilename;
    if(args.read("-o", outputFilename))
    {
        std::string ext = osgDB::getFileExtension(outputFilename);

        osg::ref_ptr<osgDB::Options> options = new osgDB::Options;

        if (ext=="osg") options->setOptionString("OutputTextureFiles");
        if (ext=="osgt") options->setOptionString("WriteImageHint=WriteOut");

        osgDB::writeNodeFile(*root, outputFilename);
        return 0;
    }


    osgViewer::Viewer viewer(args);

    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        double animationSpeed = 1.0;
        while(args.read("--speed",animationSpeed) ) {}
        char keyForAnimationPath = '5';

        std::string pathfile;
        while (args.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                apm->setTimeScale(animationSpeed);

                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    viewer.setSceneData(root);

    return viewer.run();
}
