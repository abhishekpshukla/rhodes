package com.rho.net;

import java.io.IOException;

import javax.microedition.io.SocketConnection;

import com.rho.RhoClassFactory;
import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.xruby.runtime.builtin.ObjectFactory;
import com.xruby.runtime.lang.*;

public class SSLSocket extends BaseSocket {

	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("SSLSocket");
	
	public SSLSocket(RubyClass c) {
		super(c);
	}
	
	public static SSLSocket alloc(RubyValue receiver) {
    	return new SSLSocket((RubyClass)receiver);
    }
	
	public void initialize(String strHost, int nPort) throws IOException
    {
		NetworkAccess na = (NetworkAccess)RhoClassFactory.getNetworkAccess();
		SocketConnection conn = na.socketConnect("ssl", strHost, nPort);
    	setConnection(conn);
    	super.initialize(strHost, nPort);
    }

	public static void initMethods(RubyClass klass) {
		
		klass.defineAllocMethod(new RubyNoArgMethod(){
			protected RubyValue run(RubyValue receiver, RubyBlock block )	{
				return SSLSocket.alloc(receiver);}
		});
		
		klass.defineMethod( "initialize", new RubyTwoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block )
			{
		    	try{
					String strHost = arg1.toStr();
					int nPort = arg2.toInt();
					((SSLSocket)receiver).initialize(strHost, nPort);
			        return receiver;
					
				}catch(Exception e)
				{
					LOG.ERROR("initialize failed.", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod("open", new RubyTwoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block) 
			{
				try{
					SSLSocket res = SSLSocket.alloc(RubyRuntime.SSLSocketClass);
					String strHost = arg1.toStr();
					int nPort = arg2.toInt();
					res.initialize(strHost, nPort);
					
					return res;
				}catch(Exception e)
				{
					LOG.ERROR("open failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.defineMethod( "closed?", new RubyNoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyBlock block )
			{
		    	try{
					boolean bRes = ((SSLSocket)receiver).is_closed();
					
					return ObjectFactory.createBoolean(bRes);
				}catch(Exception e)
				{
					LOG.ERROR("closed? failed.", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.defineMethod( "close", new RubyNoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyBlock block )
			{
		    	try{
					((SSLSocket)receiver).close();
					
					return RubyConstant.QNIL;
				}catch(Exception e)
				{
					LOG.ERROR("close failed.", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.defineMethod( "write", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg, RubyBlock block )
			{
		    	try{
					int nRes = ((SSLSocket)receiver).write(arg.toStr());
					
					return ObjectFactory.createInteger(nRes);
				}catch(Exception e)
				{
					LOG.ERROR("write failed.", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.defineMethod( "sysread", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg, RubyBlock block )
			{
		    	try{
					String strRes = ((SSLSocket)receiver).sysread(arg.toInt());
					
					return ObjectFactory.createString(strRes);
				}catch(Exception e)
				{
					if (e instanceof RubyException )
					{
						RubyException re = (RubyException)e;
						if ( re.getRubyValue().getRubyClass() == RubyRuntime.EOFErrorClass )
							throw re;
					}
					
					LOG.ERROR("sysread failed.", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
	}
	
}
