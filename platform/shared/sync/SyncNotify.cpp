#include "SyncNotify.h"
#include "SyncEngine.h"
#include "net/URI.h"
#include "ruby/ext/rho/rhoruby.h"

namespace rho {
namespace sync {
IMPLEMENT_LOGCLASS(CSyncNotify,"Sync");
using namespace rho::net;
using namespace rho::common;
using namespace rho::db;

String CSyncNotify::m_strObjectNotifyUrl;
common::CMutex CSyncNotify::m_mxObjectNotify;

INetRequest& CSyncNotify::getNet(){ return getSync().getNet(); }
CDBAdapter& CSyncNotify::getDB(){ return getSync().getDB(); }

void CSyncNotify::addObjectNotify(int nSrcID, const String& strObject)
{
    synchronized(m_mxObjectNotify)
    {
        Hashtable<String,int>* phashObject = m_hashSrcIDAndObject.get(nSrcID);
        if ( phashObject == null )
        {
            phashObject = new Hashtable<String,int>();
            m_hashSrcIDAndObject.put( nSrcID, phashObject );
        }

        Hashtable<String,int>& hashObject = *phashObject;
        hashObject.put(strObject,enNone);
    }
}

void CSyncNotify::addObjectNotify(const String& strSrcName, const String& strObject )
{
    synchronized(m_mxObjectNotify)
    {
        m_strSingleObjectSrcName = strSrcName;
        m_strSingleObjectID = strObject[0] == '{' ? strObject.substr(1,strObject.length()-2) : strObject ;
    }
}

void CSyncNotify::cleanObjectNotifications()
{
    synchronized(m_mxObjectNotify)
    {
        m_strSingleObjectSrcName = "";
        m_strSingleObjectID = "";
        m_hashSrcIDAndObject.clear();
    }
}

void CSyncNotify::cleanCreateObjectErrors()
{
    synchronized(m_mxObjectNotify)
    {
        m_hashCreateObjectErrors.clear();
    }
}

void CSyncNotify::processSingleObject()
{
    if ( m_strSingleObjectSrcName.length() == 0 )
        return;

    CSyncSource* pSrc = getSync().findSourceByName(m_strSingleObjectSrcName);
    if ( pSrc )
    {
        int nSrcID = pSrc->getID();
        addObjectNotify(nSrcID,m_strSingleObjectID);
    }

    m_strSingleObjectSrcName = "";
    m_strSingleObjectID = "";
}

void CSyncNotify::fireObjectsNotification()
{
	String strBody = "";
    String strUrl = "";

    {
        synchronized(m_mxObjectNotify)

        if ( m_strObjectNotifyUrl.length() == 0 )
            return;

        strUrl = getNet().resolveUrl(m_strObjectNotifyUrl);

        for (HashtablePtr<int, Hashtable<String,int>* >::iterator it = m_hashSrcIDAndObject.begin();  it != m_hashSrcIDAndObject.end(); ++it )
        {
            int nSrcID = it->first;
            Hashtable<String,int>& hashObject = *(it->second);
            for ( Hashtable<String,int>::iterator itObject = hashObject.begin();  itObject != hashObject.end(); ++itObject )
            {
                int nNotifyType = itObject->second;
                if (nNotifyType == enNone)
                    continue;

                if ( nNotifyType == enDelete )
                {
                    DBResult( res , getDB().executeSQL("SELECT object FROM object_values where object=? LIMIT 1 OFFSET 0", itObject->first ));
                    if ( !res.isEnd() )
                        nNotifyType = enUpdate;    
                }

                if ( strBody.length() > 0 )
                    strBody += "&rho_callback=1&";

                switch(nNotifyType)
                {
                case enDelete:
                    strBody += "deleted[][object]=" + itObject->first;
                    strBody += "&deleted[][source_id]=" + convertToStringA(nSrcID);
                    break;
                case enUpdate:
                    strBody += "updated[][object]=" + itObject->first;
                    strBody += "&updated[][source_id]=" + convertToStringA(nSrcID);
                    break;
                case enCreate:
                    strBody += "created[][object]=" + itObject->first;
                    strBody += "&created[][source_id]=" + convertToStringA(nSrcID);
                    break;
                }

                hashObject.put(itObject->first, enNone);
            }
        }

        if ( strBody.length() == 0 )
            return;
    }

    callNotify(strUrl, strBody);
}

void CSyncNotify::onObjectChanged(int nSrcID, const String& strObject, int nType)
{
    synchronized(m_mxObjectNotify)
    {
        processSingleObject();

        Hashtable<String,int>* phashObject = m_hashSrcIDAndObject.get(nSrcID);
        if ( phashObject == null )
            return;

        Hashtable<String,int>& hashObject = *phashObject;
        if( hashObject.containsKey(strObject) )
            hashObject.put(strObject,nType);
    }
}

void CSyncNotify::addCreateObjectError(int nSrcID, const String& strObject, const String& strError)
{
    synchronized(m_mxObjectNotify)
    {
        Hashtable<String,String>* phashErrors = m_hashCreateObjectErrors.get(nSrcID);
        if ( phashErrors == null )
        {
            phashErrors = new Hashtable<String,String>();
            m_hashCreateObjectErrors.put( nSrcID, phashErrors );
        }

        Hashtable<String,String>& hashErrors = *phashErrors;
        hashErrors.put(strObject,strError);
    }
}

String CSyncNotify::makeCreateObjectErrorBody(int nSrcID)
{
    String strBody = "";

    synchronized(m_mxObjectNotify)
    {
        Hashtable<String,String>* phashErrors = m_hashCreateObjectErrors.get(nSrcID);
        if ( phashErrors == null )
            return "";

        Hashtable<String,String>& hashErrors = *phashErrors;
        for ( Hashtable<String,String>::iterator itError = hashErrors.begin();  itError != hashErrors.end(); ++itError )
        {
            strBody += "&create_error[][object]=" + itError->first;
            strBody += "&create_error[][error_message]=" + itError->second;
        }

        hashErrors.clear();
    }

    return strBody;
}

void CSyncNotify::setObjectNotifyUrl(String strUrl)
{ 
    synchronized(m_mxObjectNotify)
    {
        m_strObjectNotifyUrl = strUrl; 
    }
}

String CSyncNotify::getObjectNotifyUrl()
{
    synchronized(m_mxObjectNotify)
    {
        return m_strObjectNotifyUrl;
    }
}

void CSyncNotify::onSyncSourceEnd( int nSrc, VectorPtr<CSyncSource*>& sources )
{
    CSyncSource& src = *sources.elementAt(nSrc);

    fireSyncNotification(&src, true, src.m_nErrCode, "");
    if ( getSync().getState() == CSyncEngine::esStop )
		fireAllSyncNotifications(true, src.m_nErrCode, "", sources );

    cleanCreateObjectErrors();
}

void CSyncNotify::setSyncNotification(int source_id, String strUrl, String strParams )
{
	LOG(INFO) + "Set notification. Source ID: " + source_id + "; Url :" + strUrl + "; Params: " + strParams;
    String strFullUrl = getNet().resolveUrl(strUrl);

	if ( source_id == -1 )
	{
		synchronized(m_mxSyncNotifications)
        {
		    m_mapSyncNotifications.clear();
    		
		    if ( strFullUrl.length() > 0 )
		    {
                DBResult( res, getDB().executeSQL("SELECT source_id from sources order by source_id") );
                for ( ; !res.isEnd(); res.next() )
		    	    m_mapSyncNotifications.put( res.getIntByIdx(0),new CSyncNotification( strFullUrl, strParams, false ) );
		    }
		    LOG(INFO) + " Done Set notification for all sources; Url :" + strFullUrl + "; Params: " + strParams;
        }
	}else
	{
        //clearSyncNotification(source_id);
        if ( strFullUrl.length() > 0 )
        {
            synchronized(m_mxSyncNotifications)
            {
                m_mapSyncNotifications.put(source_id,new CSyncNotification( strFullUrl, strParams, true ) );

		        LOG(INFO) + " Done Set notification. Source ID: " + source_id + "; Url :" + strFullUrl + "; Params: " + strParams;
            }
        }
    }
}

void CSyncNotify::setSearchNotification(int source_id, String strUrl, String strParams )
{
	LOG(INFO) + "Set search notification. Source ID: " + source_id + "; Url :" + strUrl + "; Params: " + strParams;
    String strFullUrl = getNet().resolveUrl(strUrl);

    if ( strFullUrl.length() > 0 )
    {
        synchronized(m_mxSyncNotifications)
        {
            m_mapSearchNotifications.put(source_id,new CSyncNotification( strFullUrl, strParams, true ) );

	        LOG(INFO) + " Done Set search notification. Source ID: " + source_id + "; Url :" + strFullUrl + "; Params: " + strParams;
        }
    }
}

void CSyncNotify::setInitialSyncNotification(String strUrl, String strParams )//throws Exception
{
    String strFullUrl = getNet().resolveUrl(strUrl);
	
	m_initialSyncNotify = CSyncNotification( strFullUrl, strParams, true );
}

void CSyncNotify::reportSyncStatus(String status, int error, String strDetails) {
    //TODO: reportStatus
	//if (m_statusListener != null) {
		//if ( strDetails.length() == 0 )
		//	strDetails = RhoRuby.getErrorText(error);
        //status += (strDetails.length() > 0 ? RhoRuby.getMessageText("details") + strDetails: "");
	//	m_statusListener.reportStatus( status, error);
	//}
	//LOG(INFO) + "Status: "+status;
}

void CSyncNotify::fireAllSyncNotifications( boolean bFinish, int nErrCode, String strMessage, VectorPtr<CSyncSource*>& sources )
{
    for( int i = 0; i < (int)sources.size(); i++ )
    {
    	doFireSyncNotification( sources.elementAt(i), bFinish, nErrCode, strMessage );
    }
}

void CSyncNotify::fireInitialSyncNotification( boolean bFinish, int nErrCode )
{
    if ( getSync().getState() == CSyncEngine::esExit )
		return;
	
	//TODO: show report
	if( nErrCode != RhoRuby.ERR_NONE)
	{
		String strMessage = RhoRuby.getMessageText("sync_failed_for") + "initial.";
		reportSyncStatus(strMessage,nErrCode,"");
	}
	
    boolean bRemoveAfterFire = bFinish;
    String strBody = "", strUrl;
	synchronized(m_mxSyncNotifications)
	{
        if ( m_initialSyncNotify.m_strUrl.length() == 0 )
            return;
        
        strUrl = m_initialSyncNotify.m_strUrl;
        strBody = "rho_callback=1";
        strBody += "&status=";
        if ( bFinish )
        {
	        if ( nErrCode == RhoRuby.ERR_NONE )
	        	strBody += "ok";
	        else
	        {
	        	if ( getSync().isStoppedByUser() )
                    nErrCode = RhoRuby.ERR_CANCELBYUSER;
	        	
	        	strBody += "error";				        	
			    strBody += "&error_code=" + convertToStringA(nErrCode);
	        }
        }
        else
        	strBody += "in_progress";
        
        if ( m_initialSyncNotify.m_strParams.length() > 0 )
            strBody += "&" + m_initialSyncNotify.m_strParams;
        
        bRemoveAfterFire = bRemoveAfterFire && m_initialSyncNotify.m_bRemoveAfterFire;
	}
	
    if ( bRemoveAfterFire )
    	clearInitialSyncNotification();
    
	LOG(INFO) +  "Fire initial notification.Url :" + strUrl + "; Body: " + strBody;
	
    if ( callNotify(strUrl, strBody) )
    	clearInitialSyncNotification();
}

void CSyncNotify::fireSyncNotification( CSyncSource* psrc, boolean bFinish, int nErrCode, String strMessage)
{
    if ( getSync().getState() == CSyncEngine::esExit )
		return;
	
	/*if( strMessage.length() > 0 || nErrCode != RhoRuby.ERR_NONE)
	{
		if ( !( psrc != null && psrc->m_strParams.length()>0) )
        {
			if ( psrc != null && strMessage.length() == 0 )
				strMessage = RhoRuby.getMessageText("sync_failed_for") + psrc->getName() + ".";
			
            reportSyncStatus(strMessage,nErrCode,psrc?psrc->m_strError:"");
        }
	}*/

	doFireSyncNotification(psrc, bFinish, nErrCode, strMessage );
}

void CSyncNotify::doFireSyncNotification( CSyncSource* psrc, boolean bFinish, int nErrCode, String strMessage)
{
	if ( psrc == null || getSync().isStoppedByUser() )
		return; //TODO: implement all sources callback

    CSyncSource& src = *psrc;
    String strBody, strUrl;
    boolean bRemoveAfterFire = bFinish;
    {
        synchronized(m_mxSyncNotifications)
        {
            CSyncNotification* pSN = src.isSearch() ? m_mapSearchNotifications.get(src.getID()) : m_mapSyncNotifications.get(src.getID());
            if ( pSN == 0 )
                return;
            CSyncNotification& sn = *pSN;

            strUrl = sn.m_strUrl;
		    strBody = "";
            strBody = "total_count=" + convertToStringA(src.getTotalCount());
            strBody += "&processed_count=" + convertToStringA(src.getCurPageCount());
            strBody += "&processed_objects_count=" + convertToStringA(getLastSyncObjectCount(src.getID()));
            strBody += "&cumulative_count=" + convertToStringA(src.getServerObjectsCount());
            strBody += "&source_id=" + convertToStringA(src.getID());
            strBody += "&source_name=" + src.getName();
            strBody += "&rho_callback=1";

            strBody += "&status=";

            if ( bFinish )
            {
                if ( nErrCode == RhoRuby.ERR_NONE )
	        	    strBody += "ok";
	            else
	            {
                    if ( getSync().isStoppedByUser() )
                        nErrCode = RhoRuby.ERR_CANCELBYUSER;

	        	    strBody += "error";				        	
			        strBody += "&error_code=" + convertToStringA(nErrCode);
		            strBody += "&error_message=";
                    URI::urlEncode(src.m_strError,strBody);
	            }

                strBody += makeCreateObjectErrorBody(src.getID());
            }
            else
        	    strBody += "in_progress";

            if ( sn.m_strParams.length() > 0 )
                strBody += "&" + sn.m_strParams;

            bRemoveAfterFire = bRemoveAfterFire && sn.m_bRemoveAfterFire;
        }
    }
    if ( bRemoveAfterFire )
        clearNotification(src);

	LOG(INFO) + "Fire notification. Source ID: " + src.getID() + "; Url :" + strUrl + "; Body: " + strBody;
	
    if ( callNotify(strUrl, strBody) )
        clearNotification(src);
}

boolean CSyncNotify::callNotify(const String& strUrl, const String& strBody )
{
    if ( getSync().isNoThreadedMode() )
    {
        const char* szName = strrchr(strUrl.c_str(), '/');
        if (!szName)
            szName = strUrl.c_str();
        else
            szName++;

        String strName = "C_";
        strName += szName;
        rho_ruby_set_const( strName.c_str(), strBody.c_str());
        return false;
    }

    NetResponse(resp,getNet().pushData( strUrl, strBody, null ));
    if ( !resp.isOK() )
        LOG(ERROR) + "Fire notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
    else
    {
        const char* szData = resp.getCharData();
        return szData && strcmp(szData,"stop") == 0;
    }

    return false;
}

void CSyncNotify::clearNotification(CSyncSource& src)
{
	LOG(INFO) + "Clear notification. Source : " + src.getName();

    synchronized(m_mxSyncNotifications)
    {
        if ( src.isSearch() )
            m_mapSearchNotifications.remove(src.getID());
        else
            m_mapSyncNotifications.remove(src.getID());
    }
}

void CSyncNotify::clearSyncNotification(int source_id) 
{
	LOG(INFO) + "Clear notification. Source ID: " + source_id;
	
    synchronized(m_mxSyncNotifications)
    {
        if ( source_id == -1 )//Clear all
            m_mapSyncNotifications.clear();
        else
            m_mapSyncNotifications.remove(source_id);
    }
}

void CSyncNotify::clearInitialSyncNotification() 
{
	LOG(INFO) + "Clear initial notification.";
	
	synchronized(m_mxSyncNotifications){
		m_initialSyncNotify = CSyncNotification();
	}
}

void CSyncNotify::cleanLastSyncObjectCount()
{
    synchronized(m_mxSyncNotifications)
    {
        m_hashSrcObjectCount.clear();
    }
}

int CSyncNotify::incLastSyncObjectCount(int nSrcID)
{
    int nCount = 0;
    synchronized(m_mxSyncNotifications)
    {
        nCount = m_hashSrcObjectCount.get(nSrcID)+1;
        m_hashSrcObjectCount.put(nSrcID,nCount);
    }

    return nCount;
}

int CSyncNotify::getLastSyncObjectCount(int nSrcID)
{
    int nCount = 0;
    synchronized(m_mxSyncNotifications)
    {
        nCount = m_hashSrcObjectCount.get(nSrcID);
    }

    return nCount;
}

void CSyncNotify::callLoginCallback(String callback, int nErrCode, String strMessage)
{
	//try{
    String strBody = "error_code=" + convertToStringA(nErrCode);
    strBody += "&error_message=";
    URI::urlEncode(strMessage, strBody);
    strBody += "&rho_callback=1";

    String strUrl = getNet().resolveUrl(callback);
    
	LOG(INFO) + "Login callback: " + callback + ". Body: "+ strBody;

    callNotify(strUrl, strBody);
	//}catch(Exception exc)
	//{
	//	LOG.ERROR("Call Login callback failed.", exc);
	//}
}

}
}
