package com.rho;

import java.util.Hashtable;

import net.rim.device.api.system.ApplicationDescriptor;
import net.rim.device.api.system.CodeModuleGroup;
import net.rim.device.api.system.DeviceInfo;
import net.rim.device.api.system.Display;
import net.rim.device.api.i18n.Locale;
import rhomobile.Alert;
import rhomobile.NativeBar;
import rhomobile.RhoPhonebook;
import rhomobile.RhodesApplication;
import rhomobile.RingtoneManager;
import rhomobile.WebView;
import rhomobile.camera.Camera;
import rhomobile.datetime.DateTimePicker;
import rhomobile.mapview.MapView;

import com.rho.db.HsqlDBStorage;
import com.rho.db.IDBStorage;
import com.rho.file.FileAccessBB;
import com.rho.file.Jsr75RAFileImpl;
import com.rho.file.PersistRAFileImpl;
import com.rho.net.SSLSocket;
import com.rho.net.TCPSocket;
import com.xruby.runtime.builtin.RubyArray;
import com.xruby.runtime.lang.RhoSupport;
import com.xruby.runtime.lang.RubyProgram;
import com.xruby.runtime.lang.RubyRuntime;

public class RhoRubyHelper implements IRhoRubyHelper {

	// WARNING!!! Be very careful when modify these lines! There was a case when
	// entire application has verification error in case if this line is not at start
	// of class. It is impossible to explain why it happened but need to be remembered
	public static final String USE_PERSISTENT = "use_persistent_storage";
	
	public static final int COVERAGE_BIS_B = 4;
	  
	public void initRubyExtensions(){
        RhoPhonebook.initMethods(RubyRuntime.PhonebookClass);
        Camera.initMethods(RubyRuntime.CameraClass);
        WebView.initMethods(RubyRuntime.WebViewClass);
        RhoConf.initMethods(RubyRuntime.RhoConfClass);
        Alert.initMethods(RubyRuntime.AlertClass);        
        DateTimePicker.initMethods(RubyRuntime.DateTimePickerClass);
        RingtoneManager.initMethods(RubyRuntime.RingtoneManagerClass);
        NativeBar.initMethods(RubyRuntime.NativeBarClass);
        TCPSocket.initMethods(RubyRuntime.TCPSocketClass);
        SSLSocket.initMethods(RubyRuntime.SSLSocketClass);
        MapView.initMethods(RubyRuntime.MapViewClass);
	}
	
	public RubyProgram createMainObject() throws Exception
	{
    	/*RhoRubyHelper helper = new RhoRubyHelper();
    	String appName = RhoSupport.getAppName();
		
		String strName = appName + ".ServeME.main";//com.xruby.runtime.lang.RhoSupport.createMainClassName("");
		
        Class c = Class.forName(strName);
        Object o = c.newInstance();
        RubyProgram p = (RubyProgram) o;
		
		return p;*/
		return new xruby.ServeME.main();
	}

	public String getPlatform() {
		return "Blackberry";
	}

	public void loadBackTrace(RubyArray backtrace) {
		//TODO:
	}

	public String getLocale()
	{
    	Locale loc = Locale.getDefault();
    	
    	String lang = loc != null ? loc.getLanguage() : "en";
		return lang;
	}
	
	public boolean hasNetwork() {
		/*if ((RadioInfo.getActiveWAFs() & RadioInfo.WAF_WLAN) != 0) {
			if (CoverageInfo.isCoverageSufficient( CoverageInfo.COVERAGE_CARRIER,RadioInfo.WAF_WLAN, false) || 
					CoverageInfo.isCoverageSufficient( CoverageInfo.COVERAGE_MDS,RadioInfo.WAF_WLAN, false) ||
					CoverageInfo.isCoverageSufficient( COVERAGE_BIS_B,RadioInfo.WAF_WLAN, false))
				return true;
		}

		if (CoverageInfo.isOutOfCoverage())
	        return false; 
		*/
		return true;
	}

	public String getDeviceId(){
		return new Integer( DeviceInfo.getDeviceId() ).toString();
    }

	public boolean isSimulator(){
		return DeviceInfo.isSimulator();
		//return false;
    }

	public String getModuleName()
	{
		return ApplicationDescriptor.currentApplicationDescriptor().getModuleName();
	}
	
	public void showLog()
	{
		synchronized ( RhodesApplication.getEventLock() ) {		
			RhodesApplication.getInstance().showLogScreen();
		}
	}
	
	static Hashtable m_appProperties = new Hashtable(); 
	public String getAppProperty(String name)
	{
		String strRes = null;
		synchronized (m_appProperties)
		{
			if ( m_appProperties.containsKey(name) )
				strRes = (String)m_appProperties.get(name);
			else	
			{
				CodeModuleGroup[] codeModule = CodeModuleGroup.loadAll();

				if ( codeModule != null )
				{
					String moduleName = ApplicationDescriptor
					   .currentApplicationDescriptor().getModuleName();
					
					for(int i = 0; i < codeModule.length; i++) 
					{
						String module = codeModule[i].getName();
						if( module.indexOf( moduleName ) != -1)
						{
							CodeModuleGroup group = codeModule[i];
							if ( group != null )
								strRes = group.getProperty(name);
							break;
						}
					}
				}
/*				
				CodeModuleGroup[] allGroups = CodeModuleGroupManager.loadAll();
				if ( allGroups != null )
				{
					String moduleName = ApplicationDescriptor
					   .currentApplicationDescriptor().getModuleName();
	
					CodeModuleGroup myGroup = null;
					for (int i = 0; i < allGroups.length; i++) {
					   if (allGroups[i].containsModule(moduleName)) {
					      myGroup = allGroups[i];
					      break;
					   	 }
					}
	
					if ( myGroup != null )
						strRes = myGroup.getProperty(name);
				} */
				
				if ( strRes == null )
					strRes = "";
				
				m_appProperties.put(name,strRes);
			}
		}
		
		return strRes;
	}

	public IDBStorage createDBStorage() {
		return new HsqlDBStorage();
	}

	public IFileAccess createFileAccess() {
		return new FileAccessBB();
	}

	public IRAFile createRAFile() {
		if (RhoConf.getInstance().getBool(USE_PERSISTENT))
			return new PersistRAFileImpl();
		else
			return new Jsr75RAFileImpl();
	}

	public int getScreenHeight() {
		return Display.getHeight();
	}

	public int getScreenWidth() {
		return Display.getWidth();
	}
}
