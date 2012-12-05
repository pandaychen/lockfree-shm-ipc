
#ifndef XML_WRAPPER_H_
#define XML_WRAPPER_H_
#pragma once

#include <list>
#include <string>

#include "tinyxml.h"

class ElementList
{
public:
    typedef std::list<TiXmlHandle>  xml_t;
    typedef xml_t::iterator  xml_itr;
    typedef xml_t::const_iterator  const_xml_itr;

public:
    ElementList();
    explicit ElementList(TiXmlHandle const& handle);

    const_xml_itr begin() const
    {
        return m_elements.begin();
    }

    xml_itr begin()
    {
        return m_elements.begin();
    }

    const_xml_itr end() const
    {
        return m_elements.end();
    }

    xml_itr end()
    {
        return m_elements.end();
    }

    bool empty() const
    {
        return m_elements.empty();
    }

    size_t size() const
    {
        return m_elements.size();
    }

private:
    xml_t    m_elements;
};

class XmlOperator
{
public:
    static TiXmlDocument* LoadXml(const char* fileName);
	
    static ElementList GetChildElements(const TiXmlDocument* doc);
	
    static ElementList GetChildElements(TiXmlHandle const& elem);
	
    static TiXmlHandle GetFirstElement(const TiXmlDocument* doc);
	
    static TiXmlHandle GetChildElement(const TiXmlDocument* doc,
                                       std::string const& tagName,
                                       int index = 0);
	
    static TiXmlHandle GetChildElement(TiXmlHandle const& elem,
                                       std::string const& tagName,
                                       int index = 0);
	
    static const char* GetElementTag(TiXmlHandle const& elem);
	
    static const char* GetAttribute(TiXmlHandle const& elem,
                                    std::string const& name);
	
    static const char* GetText(TiXmlHandle const& elem);
	
    static bool HaveChildElems(TiXmlHandle const& elem);
	
    static bool IsNullHandle(TiXmlHandle const& elem);
	
};

#endif // XML_WRAPPER_H_

