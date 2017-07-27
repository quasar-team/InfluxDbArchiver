/*
 * InfluxDbArchiver.cpp
 *
 *  Created on: 2 Jun 2017
 *      Author: pnikiel
 */

#include <boost/lexical_cast.hpp>

#include <InfluxDbArchiver.h>

#include <curl/curl.h>

#include <LogIt.h>
#include <ASUtils.h>

namespace InfluxDbArchiver
{

ArchivedItem::ArchivedItem( const std::string& attribute, const std::string& addr, const std::string& value ):
        m_attribute(attribute),
        m_address(addr),
        m_value(value)
{

        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts );
        int64_t timestamp = (int64_t)(ts.tv_sec) * (int64_t)1000000000 + (int64_t)(ts.tv_nsec);

        m_timestamp = boost::lexical_cast<std::string>( timestamp );

}

InfluxDbArchiver::InfluxDbArchiver(
        const std::string& db,
        const std::string& url )
    {

        // singleton: let only one specific archiver in the system
        if (s_instance)
            abort_with_message(__FILE__,__LINE__,"Trying to create more then one specific archiver objects. Can't - it's a singleton.");

        s_instance = this;

        m_handle = curl_easy_init(  );
        m_url = url + "/write?db=" + db;
        curl_easy_setopt(m_handle, CURLOPT_URL, m_url.c_str());
        m_archiverThread = boost::thread([this](){ this->archivingThread(); });

        m_isRunning = true;
    }

void InfluxDbArchiver::archiveAssignment ( const UaNodeId& objectAddress, const UaNodeId& variableAddress, OpcUa_UInt32 value, UaStatus statusCode  )
{
    boost::lock_guard<decltype (m_lock)> lock (m_lock);
    m_pendingItems.emplace_back( "value", variableAddress.toString().toUtf8(), boost::lexical_cast<std::string>(value) );
}

void InfluxDbArchiver::archivingThread ()
{
    LOG(Log::INF) << " in archiving thread!";
    while(m_isRunning)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
        {
            boost::lock_guard<decltype (m_lock)> lock (m_lock);
            unsigned int numItems = m_pendingItems.size();
            LOG(Log::INF) << "queued items: " << numItems;
            if (numItems > 0)
            {
                std::string influxLine = "";
                for( const ArchivedItem& ai: m_pendingItems  )
                {
                    influxLine += ai.attribute()+",addr="+ai.address()+" value="+ai.value()+" "+ai.timestamp()+"\n";
                }
                curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, influxLine.c_str());
                CURLcode code = curl_easy_perform(m_handle); /* post away! */
                LOG(Log::INF) << "CURL result code was: " << code;
                m_pendingItems.clear();
            }
        }
    }
}

void InfluxDbArchiver::kill ()
{
    LOG(Log::INF) << "InfluxDB archiver exiting ... ";
    m_isRunning = false;
    m_archiverThread.join();
}


}
