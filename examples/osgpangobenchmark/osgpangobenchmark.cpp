// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/TextTransform>
#include <osgPango/ShaderManager>

class ConvertPangoText : public osg::NodeVisitor
{
public:
    ConvertPangoText(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
        _groupStack.push_back(new osg::Group);
    }

    void pushNode(osg::Node* node)
    {
        _groupStack.back()->addChild(node);
        if (node->asGroup()) _groupStack.push_back(node->asGroup());
    }

    void popNode(osg::Node* node)
    {
        if (node->asGroup()) _groupStack.pop_back();
    }

    void apply(osg::Node& node)
    {
        OSG_NOTICE<<"Node from "<<node.className()<<std::endl;

        osg::ref_ptr<osg::Node> new_node = osg::cloneType(&node);

        pushNode(new_node.get());

        if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

        traverse(node);

        popNode(new_node.get());
    }

    void apply(osg::Geode& node)
    {
        OSG_NOTICE<<"Geode from "<<node.className()<<std::endl;

        osg::ref_ptr<osg::Node> new_node = new osg::Geode;

        pushNode(new_node.get());

        if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

        traverse(node);

        popNode(new_node.get());
    }

    void apply(osg::Group& node)
    {
        OSG_NOTICE<<"Group from "<<node.className()<<std::endl;

        osg::ref_ptr<osg::Group> new_node = new osg::Group;

        pushNode(new_node.get());

        if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

        traverse(node);

        popNode(new_node.get());
    }

    void apply(osg::MatrixTransform& node)
    {
        osg::ref_ptr<osg::MatrixTransform> new_node = new osg::MatrixTransform;

        pushNode(new_node.get());

        new_node->setMatrix(node.getMatrix());

        if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

        OSG_NOTICE<<"MatrixTransform from "<<node.className()<<std::endl;

        traverse(node);

        popNode(new_node.get());
    }

    void apply(osg::Geometry& geometry)
    {
        if (geometry.getStateSet()) apply(geometry.getStateSet());

        OSG_NOTICE<<"osg::Geometry "<<std::endl;

        osg::ref_ptr<osg::Geometry> new_geometry = new osg::Geometry(geometry);

        _groupStack.back()->addChild(new_geometry);
    }

    void apply(osg::StateSet* stateset)
    {
        for(osg::StateSet::TextureAttributeList::iterator titr = stateset->getTextureAttributeList().begin();
            titr != stateset->getTextureAttributeList().end();
            ++titr)
        {
            for(osg::StateSet::AttributeList::iterator aitr = titr->begin();
                aitr != titr->end();
                ++aitr)
            {
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(aitr->second.first.get());
                if (texture)
                {
                    osg::Image* image = texture->getImage();
                }
            }
        }
    }

    osg::ref_ptr<osg::Node> getSubgraph()
    {
        return _groupStack.empty() ? 0 : _groupStack.front().get();
    }

    typedef std::vector< osg::ref_ptr<osg::Group> > GroupStack;
    GroupStack _groupStack;
};

class ProcessRoads : public osg::NodeVisitor
{
public:
    ProcessRoads(const std::string& renderer, const osgPango::TextOptions& to, double labelHeight, double labelSpacing):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _renderer(renderer),
        _textOptions(to),
        _labelHeight(labelHeight),
        _labelSpacing(labelSpacing)
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
    double computeLength(T* vertices, osg::DrawArrays* prim)
    {
        if (prim->getCount()==0) return 0.0;

        unsigned int last = prim->getFirst()+prim->getCount()-1;
        double length = 0.0;

        for(unsigned int i = prim->getFirst(); i<last; ++i)
        {
            length += ((*vertices)[i+1]-(*vertices)[i]).length();
        }

        return length;
    }

    osgPango::TextTransform* createText(const std::string& name)
    {
        osgPango::TextTransform* t = new osgPango::TextTransform;
        t->setGlyphRenderer(_renderer);
        t->setAxisAlignment(osgPango::TextTransform::AXIS_ALIGN_XZ_PLANE);
        t->setText(name, _textOptions);
        t->finalize();
        return t;
    }

    void scaleAndPositionText(osgPango::TextTransform* text, const osg::Vec3d& position)
    {
        OSG_NOTICE<<"scaleAndPositionText("<<text<<") position = "<<position<<" size="<<text->getSize()<<std::endl;
        osg::Vec2 size = text->getSize();
        double scale = _labelHeight/size.y();
        text->setMatrix(osg::Matrix::scale(scale, scale, scale) * osg::Matrix::translate(position));
    }

    void createRoadLabel(const std::string& name, osg::Array* vertices, osg::PrimitiveSet* prim)
    {
        osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*>(prim);
        osg::Vec3Array* vetices_float = dynamic_cast<osg::Vec3Array*>(vertices);
        osg::Vec3dArray* vetices_double = dynamic_cast<osg::Vec3dArray*>(vertices);
        bool use_float = vetices_float!=0;

        if (da)
        {
            unsigned int first = da->getFirst();
            osg::Vec3d first_vertex = use_float ? osg::Vec3d((*vetices_float)[first]) : (*vetices_double)[first];
            double length = use_float ? computeLength(vetices_float, da) : computeLength(vetices_double, da);
            OSG_NOTICE<<"createRoadLabel("<<name<<", "<<first_vertex<<") length = "<<length<<std::endl;


            osgPango::TextTransform* text = createText(name);
            scaleAndPositionText(text, first_vertex);

            _labelSubgraph->addChild(text);

        }
    }

    osg::ref_ptr<osg::Node> getRoadLabels()
    {
        return _labelSubgraph;
    }

    typedef std::vector<std::string> NameStack;
    NameStack _nameStack;


    std::string                 _renderer;
    osgPango::TextOptions       _textOptions;
    double                      _labelHeight;
    double                      _labelSpacing;

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

    while (args.read("--road", filename))
    {
        double labelHeight = 1.0;
        while (args.read("--label-height", labelHeight)) {}

        double labelSpacing = 100.0;
        while (args.read("--label-spacing", labelSpacing)) {}


        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(filename);
        if (node)
        {
            ProcessRoads processRoads(renderer, to, labelHeight, labelSpacing);
            node->accept(processRoads);

            group->addChild(node);

            osg::ref_ptr<osg::Node> roadLabels = processRoads.getRoadLabels();
            if (roadLabels)
            {
                assignedText = false;
                group->addChild(roadLabels);

                //root = roadLabels;
            }
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
        ConvertPangoText convertPangoText;
        root->accept(convertPangoText);

        root = convertPangoText.getSubgraph();
    }

    if (args.read("--optimize"))
    {
        osgUtil::Optimizer optimizer;
        optimizer.optimize(root);
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

    viewer.setSceneData(root);

    return viewer.run();
}
