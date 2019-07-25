//////////////////////////////////////////////////////////////////////////////////
/**
 * @file         SoapParser.cpp
 * @author       Manjesh Shivakumar
 * @short        This is the source file for SOAP Parser functionality of RP1Broker 
 *
 * Copyright (c) 2019 Nokia. All rights reserved.
 *
 **/
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include<cstring>
#include"SoapParser.hpp"

const std::string soapMsgId {"id"};
const std::string soapMsgHeader {"Header"};
const std::string soapMsgRelatesTo {"relatesTo"};
const std::string soapMsgMo {"managedObject"};
const std::string soapMsgMoClass {"class"};
const std::string soapMsgMoDistName {"distName"};
const std::string soapMsgBody {"Body"};
const std::string soapMsgParamName{"parameterName"};
const std::string soapMsgNewVal{"newValue"};

RP1Broker::SoapParser::SoapParser(std::string msg)
{
    populateSoapMsgDetails(msg);
}

RP1Broker::SoapParser::~SoapParser()
{

}

void RP1Broker::SoapParser::getSoapMsgMoList(xmlNode* node)
{
    xmlChar* val = NULL;
    bool isClass = false, isDistName = false;
    std::string str;
    xmlAttr* attr = node->properties;

    while(attr)
    {
        if(0 == strcmp(soapMsgMoClass.c_str(), (const char*)attr->name))
            isClass = true;

	if(0 == strcmp(soapMsgMoDistName.c_str(), (const char*)attr->name))
            isDistName = true;

        attr = attr->next;
    }

    attr = node->properties;
    while(attr)
    {
        if((true == isClass) && (0 == strcmp(soapMsgMoClass.c_str(), (const char*)attr->name)))
        {
            val = xmlNodeListGetString(node->doc, attr->children, 1);
	    if(NULL == val)
                throw SoapParserException("getSoapMsgMoList::Building the string equivalent to the text contained in the Node list failed");
	    str = (char*)val;
	    msgMoListVal.push_back(str);
	    xmlFree(val);
	    break;
        }

	if((false == isClass) && (true == isDistName) && (0 == strcmp(soapMsgMoDistName.c_str(), (const char*)attr->name)))
        {
            val = xmlNodeListGetString(node->doc, attr->children, 1);
	    if(NULL == val)
                throw SoapParserException("getSoapMsgMoList::Building the string equivalent to the text contained in the Node list failed");
            str = (char*)val;
            msgMoListVal.push_back(str);
            xmlFree(val);
            break;
        }
        attr = attr->next;
    }
}

void RP1Broker::SoapParser::getSoapMsgDetails(xmlNode* node, int indentLen)
{
    while(node)
    {
        if(XML_ELEMENT_NODE == node->type)
        {
            if(0 == strcmp(soapMsgRelatesTo.c_str(), (const char*)node->name))
	    {
	        relatesToVal = (char*)xmlNodeGetContent(node);
		if(relatesToVal.empty())
                    throw SoapParserException("getSoapMsgDetails::Unable to fetch relatesTo field");
	    }

#ifdef TESTING
	    if(0 == strcmp(soapMsgId.c_str(), (const char*)node->name))
            {
                msgIdVal = (char*)xmlNodeGetContent(node);
		if(msgIdVal.empty())
                    throw SoapParserException("getSoapMsgDetails::Unable to fetch id field");
            }
	    if(0 == strcmp(soapMsgParamName.c_str(), (const char*)node->name))
            {
                msgParamName = (char*)xmlNodeGetContent(node);
            }
            if(0 == strcmp(soapMsgNewVal.c_str(), (const char*)node->name))
            {
                msgNewVal = (char*)xmlNodeGetContent(node);
            }
#endif

            if(0 == strcmp(soapMsgMo.c_str(), (const char*)node->name))
	    {
                getSoapMsgMoList(node);
	    }

            if(0 == strcmp(soapMsgBody.c_str(), (const char*)node->name))
	    {
                node = node->children;
                node = node->next;
		msgTypeVal = (char*)node->name;
                node = node->prev;
                node = node->parent;
            }
        }
        getSoapMsgDetails(node->children, ++indentLen);
        node = node->next;
    }
}

void RP1Broker::SoapParser::insertSoapMsgId(xmlDoc* doc, xmlNode * node, int indentLen, const std::string soapId)
{
    xmlNode* node_name = NULL;
    xmlNode* node_val = NULL;
   
    while(node)
    {
        if(XML_ELEMENT_NODE == node->type)
        {
            if(0 == strcmp(soapMsgHeader.c_str(), (const char*)node->name))
	    {
		node_name = xmlNewNode(NULL, BAD_CAST soapMsgId.c_str());
		if(NULL == node_name)
	            throw SoapParserException("insertSoapMsgId::Creation of new node element failed");	   
                node_val = xmlNewText(BAD_CAST soapId.c_str());
		if (NULL == node_val)
		{
	            xmlFreeNode(node_name);
		    throw SoapParserException("insertSoapMsgId::Creation of a new text node failed");
		}
		if ( (not xmlAddChild(node_name, node_val)) or (not xmlAddChild(node, node_name)) )
                {
                    xmlFreeNode(node_name);
		    xmlFreeNode(node_val);
                    throw SoapParserException("insertSoapMsgId::Adding a new node failed");
                }
            }
        }
        insertSoapMsgId(doc, node->children, ++indentLen, soapId);
        node = node->next;
    }
}

void RP1Broker::SoapParser::populateSoapMsgDetails(std::string msg)
{
    xmlDoc* doc = NULL;
    xmlNode* root = NULL;

    std::cout << "msg : " << msg.c_str() << std::endl;
    try
    {
        doc = xmlReadMemory(msg.c_str(), msg.length(), NULL, NULL, 0);
        if(NULL == doc)
        {
	    throw SoapParserException("populateSoapMsgDetails::Parsing xml in-memory failed");
        }

        root = xmlDocGetRootElement(doc);
        if(NULL == root)
        {
            throw SoapParserException("populateSoapMsgDetails::Getting the root element failed");
        }

        getSoapMsgDetails(root, 1);
    }
    catch(const  RP1Broker::SoapParserException &exp)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        throw SoapParserException(exp.what());
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

void RP1Broker::SoapParser::addIdToSoapMsg(const std::string soapId, std::string& msg)
{
    xmlDoc* doc = NULL;
    xmlNode* root = NULL;
    xmlChar* buf = NULL;
    int len = 0;

    try
    {
        doc = xmlReadMemory(msg.c_str(), msg.length(), NULL, NULL, 0);
        if(NULL == doc)
        {
            throw SoapParserException("addIdToSoapMsg::Parsing xml in-memory failed");
        }

        root = xmlDocGetRootElement(doc);
        if(NULL == root)
        {
            throw SoapParserException("addIdToSoapMsg::Getting the root element failed");
        }

        insertSoapMsgId(doc, root, 1, soapId);
        xmlDocDumpMemoryEnc(doc, &buf, &len, "UTF-8");
        msg = (char*)buf;
    }
    catch(const  RP1Broker::SoapParserException &exp)
    {
	xmlFree(buf);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        throw SoapParserException(exp.what());
    }
    xmlFree(buf);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

void RP1Broker::SoapParser::getRelatesTo(std::string& relatesTo)
{
    relatesTo = relatesToVal;
}

void RP1Broker::SoapParser::getMsgType(std::string& msgType)
{
    msgType = msgTypeVal;
}

void RP1Broker::SoapParser::getMsgMoList(std::list<std::string>& msgMoList)
{
    msgMoList = msgMoListVal;
}

#ifdef TESTING
void RP1Broker::SoapParser::getMsgId(std::string& msgId)
{
    msgId = msgIdVal;
}

void RP1Broker::SoapParser::getMsgParamName(std::string& msgParamNm)
{
    msgParamNm = msgParamName;
}

void RP1Broker::SoapParser::getMsgNewVal(std::string& msgNewValue)
{
    msgNewValue = msgNewVal;
}

void RP1Broker::SoapParser::addRelatesToSoapMsg(const std::string filePath,const std::string msgId, std::string& msg)
{
    xmlDoc* doc = NULL;
    xmlNode* root = NULL;
    xmlChar* buf = NULL;
    int len = 0;

    try
    {
        doc = xmlReadFile(filePath.c_str(), NULL, 0);
        if(NULL == doc)
        {
            throw SoapParserException("addRelatesToSoapMsg::Parsing xml in-memory failed");
        }

        root = xmlDocGetRootElement(doc);
        if(NULL == root)
        {
            throw SoapParserException("addRelatesToSoapMsg::Getting the root element failed");
        }

        insertSoapMsgRelatesTo(doc, root, 1, msgId);
        xmlDocDumpMemoryEnc(doc, &buf, &len, "UTF-8");
        msg = (char*)buf;
    }
    catch(const  RP1Broker::SoapParserException &exp)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        throw SoapParserException(exp.what());
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

void RP1Broker::SoapParser::insertSoapMsgRelatesTo(xmlDoc* doc, xmlNode * node, int indentLen, const std::string rrhId)
{
    xmlNode* node_name = NULL;
    xmlNode* node_val = NULL;

    while(node)
    {
        if(XML_ELEMENT_NODE == node->type)
        {
            if(0 == strcmp(soapMsgHeader.c_str(), (const char*)node->name))
            {
                node_name = xmlNewNode(NULL, BAD_CAST soapMsgRelatesTo.c_str());
                if(NULL == node_name)
                    throw SoapParserException("insertSoapMsgRelatesTo::Creation of new node element failed");

                node_val = xmlNewText(BAD_CAST rrhId.c_str());
                if (NULL == node_val)
                {
                    xmlFreeNode(node_name);
                    throw SoapParserException("insertSoapMsgRelatesTo::Creation of a new text node failed");
                }

                if ( (not xmlAddChild(node_name, node_val)) or (not xmlAddChild(node, node_name)) )
                {
                    xmlFreeNode(node_name);
                    xmlFreeNode(node_val);
                    throw SoapParserException("insertSoapMsgRelatesTo::Adding a new node failed");
                }
            }
        }
        insertSoapMsgRelatesTo(doc, node->children, ++indentLen,rrhId);
        node = node->next;
    }
}
#endif
