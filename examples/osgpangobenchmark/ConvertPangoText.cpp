#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Texture2D>

#include "ConvertPangoText.h"

ConvertPangoText::ConvertPangoText(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    _groupStack.push_back(new osg::Group);
}

void ConvertPangoText::pushNode(osg::Node* node)
{
    _groupStack.back()->addChild(node);
    if (node->asGroup()) _groupStack.push_back(node->asGroup());
}

void ConvertPangoText::popNode(osg::Node* node)
{
    if (node->asGroup()) _groupStack.pop_back();
}

void ConvertPangoText::apply(osg::Node& node)
{
    // OSG_NOTICE<<"Node from "<<node.className()<<std::endl;

    osg::ref_ptr<osg::Node> new_node = osg::cloneType(&node);

    pushNode(new_node.get());

    if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

    traverse(node);

    popNode(new_node.get());
}

void ConvertPangoText::apply(osg::Geode& node)
{
    // OSG_NOTICE<<"Geode from "<<node.className()<<std::endl;

    osg::ref_ptr<osg::Node> new_node = new osg::Geode;

    pushNode(new_node.get());

    if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

    traverse(node);

    popNode(new_node.get());
}

void ConvertPangoText::apply(osg::Group& node)
{
    // OSG_NOTICE<<"Group from "<<node.className()<<std::endl;

    osg::ref_ptr<osg::Group> new_node = new osg::Group;

    pushNode(new_node.get());

    if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

    traverse(node);

    popNode(new_node.get());
}

void ConvertPangoText::apply(osg::MatrixTransform& node)
{
    osg::ref_ptr<osg::MatrixTransform> new_node = new osg::MatrixTransform;

    pushNode(new_node.get());

    new_node->setDataVariance(osg::Object::STATIC);
    new_node->setMatrix(node.getMatrix());

    if (node.getStateSet()) new_node->setStateSet(node.getStateSet());

    // OSG_NOTICE<<"MatrixTransform from "<<node.className()<<std::endl;

    traverse(node);

    popNode(new_node.get());
}

void ConvertPangoText::apply(osg::Geometry& geometry)
{
    if (geometry.getStateSet()) apply(geometry.getStateSet());

    // OSG_NOTICE<<"osg::Geometry "<<std::endl;

    osg::ref_ptr<osg::Geometry> new_geometry = new osg::Geometry(geometry);

    new_geometry->setDataVariance(osg::Object::STATIC);

    _groupStack.back()->addChild(new_geometry);
}

void ConvertPangoText::apply(osg::StateSet* stateset)
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

osg::ref_ptr<osg::Node> ConvertPangoText::getSubgraph()
{
    return _groupStack.empty() ? 0 : _groupStack.front().get();
}
