#ifndef _RHOFILEPATH_H_
#define _RHOFILEPATH_H_

#include "RhoPort.h"

namespace rho{
namespace general{

class CFilePath{
public:
    CFilePath(const char* path) : m_szPath(path){}

    const char* getBaseName(){ 
        const char* base = findLastSlash();
        if (base)
            return base+1;

        return m_szPath;
    }

    String makeFullPath(const char* szFileName){
        String res = m_szPath;
        if ( !findLastSlash() )
            res += "/";

        res += szFileName;
        
        return res;
    }

private:

    const char* findLastSlash(){
        const char* slash = strrchr(m_szPath, '/');
        if ( !slash )
            slash = strrchr(m_szPath, '\\');

        return slash;
    }

    const char* m_szPath;
};

}
}

#endif //_RHOFILEPATH_H_