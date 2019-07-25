//////////////////////////////////////////////////////////////////////////////////
/**
 * @file         SoapParser.hpp
 * @author       Manjesh Shivakumar
 * @short        This is the header file for SOAP Parser functionality of RP1Broker 
 *
 * Copyright (c) 2019 Nokia. All rights reserved.
 *
 **/
////////////////////////////////////////////////////////////////////////////////

#ifndef RP1BROKER_SOAPPARSER_HPP
#define RP1BROKER_SOAPPARSER_HPP
#include<list>
#include<string>
#include<libxml/tree.h>
#include<libxml/parser.h>

namespace RP1Broker
{   
    class SoapParserException : public std::exception {
        public:
            SoapParserException(std::string message) {
                reason = message;
            }
            virtual ~SoapParserException(){}
            virtual const char* what() const noexcept override {
                return reason.c_str();
            }
        private:
            std::string reason;
    };

    class SoapParser
    {
        private:
	    std::list<std::string> msgMoListVal;
	    std::string msgTypeVal;
	    std::string relatesToVal;
#ifdef TESTING
	    std::string msgIdVal;
	    std::string msgParamName;
            std::string msgNewVal;
#endif

	    void getSoapMsgMoList(xmlNode* node);
	    void getSoapMsgDetails(xmlNode* node, int indentLen);
	    void populateSoapMsgDetails(std::string msg);

        public:
	    SoapParser(std::string msg);
	    virtual ~SoapParser();
            void getRelatesTo(std::string& relatesTo);
	    void getMsgType(std::string& msgType);
	    void getMsgMoList(std::list<std::string>& msgMoList);
	    static void addIdToSoapMsg(const std::string soapId, std::string& msg);
	    static void insertSoapMsgId(xmlDoc* doc, xmlNode * node, int indentLen, const std::string soapId);
#ifdef TESTING
	    void getMsgId(std::string& msgType);
	    void getMsgParamName(std::string& msgParamName);
            void getMsgNewVal(std::string& msgNewVal);
	    static void addRelatesToSoapMsg(const std::string filePath,const std::string msgId, std::string& msg);
	    static void insertSoapMsgRelatesTo(xmlDoc* doc, xmlNode * node, int indentLen, const std::string rrhId);
#endif
    };
}
#endif
