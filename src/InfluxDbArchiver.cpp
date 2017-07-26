/*
 * InfluxDbArchiver.cpp
 *
 *  Created on: 2 Jun 2017
 *      Author: pnikiel
 */

#include <InfluxDbArchiver.h>

#include <curl/curl.h>

#include <LogIt.h>

namespace InfluxDbArchiver
{

InfluxDbArchiver::InfluxDbArchiver(
        const std::string& db,
        const std::string& url )
    {
        m_handle = curl_easy_init(  );
        m_url = url + "/write?db=" + db;
        curl_easy_setopt(m_handle, CURLOPT_URL, m_url.c_str());
        m_archiverThread = boost::thread([this](){ this->archivingThread(); });
    }

void InfluxDbArchiver::archivingThread ()
{
    LOG(Log::INF) << " in archiving thread!";
    while(1)
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


}
