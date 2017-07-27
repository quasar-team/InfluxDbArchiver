/*
 * InfluxDbArchiver.h
 *
 *  Created on: 2 Jun 2017
 *      Author: pnikiel
 */

#ifndef INFLUXDBARCHIVER_INCLUDE_INFLUXDBARCHIVER_H_
#define INFLUXDBARCHIVER_INCLUDE_INFLUXDBARCHIVER_H_


#include <GenericArchiver.h>

#include <string>
#include <list>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

typedef void CURL; // forward-def

namespace InfluxDbArchiver
{




class ArchivedItem
{

public:
    ArchivedItem( const std::string& attribute, const std::string& addr, const std::string& value );

    std::string attribute() const { return m_attribute; }
    std::string address() const { return m_address; }
    std::string value () const { return m_value; }
    std::string timestamp() const { return m_timestamp; }

private:
    std::string m_attribute;
    std::string m_address;
    std::string m_value;
    std::string m_timestamp;
};

class InfluxDbArchiver: public GenericArchiver::GenericArchiver
{
public:

    InfluxDbArchiver(
            const std::string& db,
            const std::string& url = "http://localhost:8086"
                    );

    void archiveAssignment ( const UaNodeId& objectAddress, const UaNodeId& variableAddress, OpcUa_UInt32 value, UaStatus statusCode  );

    void archivingThread ();

    virtual void kill ();

private:
    CURL* m_handle;
    std::list<ArchivedItem> m_pendingItems;
    boost::mutex m_lock;
    boost::thread m_archiverThread;
    std::string m_url;
    bool m_isRunning;

};

}


#endif /* INFLUXDBARCHIVER_INCLUDE_INFLUXDBARCHIVER_H_ */
