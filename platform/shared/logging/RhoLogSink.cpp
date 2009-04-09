#include "RhoLogSink.h"

#include "common/RhoFile.h"
#include "common/StringConverter.h"

#if defined( OS_SYMBIAN )
#include <e32debug.h>
#endif

namespace rho {

CLogFileSink::CLogFileSink(const LogSettings& oSettings) : m_oLogConf(oSettings), m_nCirclePos(-1),
    m_pFile(0), m_pPosFile(0), m_nFileLogSize(0)
{

}

void CLogFileSink::writeLogMessage( const char* data, unsigned int len ){
    if ( !m_pFile )    
        m_pFile = new general::CRhoFile();

    if ( !m_pFile->isOpened() ){
        m_pFile->open( getLogConf().getLogFilePath().c_str(), general::CRhoFile::OpenForAppend );
        m_nFileLogSize = m_pFile->size();
        loadLogPosition();
    }

    if ( getLogConf().getMaxLogFileSize() > 0 )
    {
        if ( ( m_nCirclePos >= 0 && m_nCirclePos + len > getLogConf().getMaxLogFileSize() ) || 
             ( m_nCirclePos < 0 && m_nFileLogSize + len > getLogConf().getMaxLogFileSize() ) )
        {
            m_pFile->movePosToStart();
            m_nFileLogSize = 0;
            m_nCirclePos = 0;
        }
    }

    int nWritten = m_pFile->write( data, len );
    m_pFile->flush();

    if ( m_nCirclePos >= 0 )
        m_nCirclePos += nWritten;
    else
        m_nFileLogSize += nWritten;

    saveLogPosition();
}

void CLogFileSink::loadLogPosition(){
    if ( !m_pPosFile )
        m_pPosFile = new general::CRhoFile();

    if ( !m_pPosFile->isOpened() ){
        String strPosPath = getLogConf().getLogFilePath() + "_pos";
        m_pPosFile->open( strPosPath.c_str(), general::CRhoFile::OpenForReadWrite );
    }

    if ( !m_pPosFile->isOpened() )
        return;

    String strPos;
    m_pPosFile->readString(strPos);
    if ( strPos.length() == 0 )
        return;

    m_nCirclePos = atoi(strPos.c_str());
    if ( m_nCirclePos >= 0 )
        m_pFile->setPosTo( m_nCirclePos );
}

void CLogFileSink::saveLogPosition(){
    if ( m_nCirclePos < 0 )
        return;

    String strPos = general::convertToStringA(m_nCirclePos);
    m_pPosFile->write( strPos.c_str(), strPos.length() );
    m_pPosFile->flush();
    m_pPosFile->movePosToStart();
}

void CLogOutputSink::writeLogMessage( const char* data, unsigned int len ){

#if defined( OS_WINDOWS ) //|| defined( OS_WINCE )
        ::OutputDebugStringA(data);
        printf(data);
#elif defined( OS_SYMBIAN )
        RDebug::Printf(data);
#else
        printf(data);
#endif

}

}