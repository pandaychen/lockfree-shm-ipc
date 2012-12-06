
#include "xml_wrapper.h"

ElementList::ElementList() 
{
}

ElementList::ElementList(TiXmlHandle const& handle)
{
    TiXmlElement* brother = handle.ToElement();

    for (; brother; brother = brother->NextSiblingElement())
    {
        m_elements.push_back(TiXmlHandle(brother));
    }
}

TiXmlDocument* XmlOperator::LoadXml(const char* fileName)
{
    TiXmlDocument* doc = new TiXmlDocument(fileName);
    if (doc->LoadFile())
    {
        return doc;
    }
    return NULL;
}

ElementList XmlOperator::GetChildElements(const TiXmlDocument* doc)
{
    if (doc->NoChildren())
    {
        return ElementList();
    }
    else
    {
        return ElementList(TiXmlHandle(const_cast< TiXmlElement* >(doc->FirstChildElement())));
    }
}

ElementList XmlOperator::GetChildElements(TiXmlHandle const& elem)
{
    if (elem.ToElement()->NoChildren())
    {
        return ElementList();
    }
    else
    {
        return ElementList(TiXmlHandle(elem.FirstChildElement().ToElement()));
    }
}

TiXmlHandle XmlOperator::GetFirstElement(const TiXmlDocument* doc)
{
    TiXmlHandle handle(const_cast<TiXmlDocument*>(doc));
    return handle.FirstChildElement();
}

TiXmlHandle XmlOperator::GetChildElement(const TiXmlDocument* doc,
                                         std::string const& tagName,
                                         int index)
{
    TiXmlHandle handle(const_cast<TiXmlDocument*>(doc));
    return handle.ChildElement(tagName.c_str(), index);
}

TiXmlHandle XmlOperator::GetChildElement(TiXmlHandle const& elem,
                                         std::string const& tagName,
                                         int index)
{
    return elem.ChildElement(tagName.c_str(), index);
}

const char* XmlOperator::GetElementTag(TiXmlHandle const& elem)
{
    return elem.ToElement()->Value();
}

const char* XmlOperator::GetAttribute(TiXmlHandle const& elem,
                                      std::string const& name)
{
    return elem.ToElement()->Attribute(name.c_str());
}

const char* XmlOperator::GetText(TiXmlHandle const& elem)
{
    return elem.ToElement()->GetText();
}

bool XmlOperator::HaveChildElems(TiXmlHandle const& elem)
{
    return !(elem.ToElement()->NoChildren());
}

bool XmlOperator::IsNullHandle(TiXmlHandle const& elem)
{
    return (NULL == elem.ToNode()) ? true : false;
}

