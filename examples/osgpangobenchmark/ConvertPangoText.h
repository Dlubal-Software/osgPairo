#ifndef ConvertPangoText_H
#define ConvertPangoText_H

#include <osg/Group>
#include <osg/NodeVisitor>

class ConvertPangoText : public osg::NodeVisitor
{
public:
    ConvertPangoText();

    void pushNode(osg::Node* node);
    void popNode(osg::Node* node);

    void apply(osg::Node& node);

    void apply(osg::Geode& node);

    void apply(osg::Group& node);

    void apply(osg::MatrixTransform& node);

    void apply(osg::Geometry& geometry);

    void apply(osg::StateSet* stateset);

    osg::ref_ptr<osg::Node> getSubgraph();

    typedef std::vector< osg::ref_ptr<osg::Group> > GroupStack;
    GroupStack _groupStack;
};

#endif
