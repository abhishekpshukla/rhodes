/**
 * Copyright 2007 Xue Yong Zhi
 * Distributed under the BSD License
 */

package com.xruby.runtime.stdlib;

import com.xruby.runtime.builtin.ObjectFactory;
import com.xruby.runtime.builtin.RubyArray;
import com.xruby.runtime.builtin.RubyString;
import com.xruby.runtime.lang.*;

//@RubyLevelClass(name="StringIO", modules="Enumerable")
public class RubyStringIO extends RubyBasic {
    private RubyString value_;
    private int nPos_ = 0;
    
    private RubyStringIO() {
        super(RubyRuntime.StringIOClass);
    }

	protected void doClone(RubyValue orig){
		value_ = ((RubyStringIO)orig).value_;
		super.doClone(orig);
	}
    
    //@RubyAllocMethod
    public static RubyStringIO alloc(RubyValue receiver) {
        return new RubyStringIO();
    }

    //@RubyLevelMethod(name="initialize")
    public RubyStringIO initialize() {
        this.value_ = ObjectFactory.createString();
        return this;
    }
    
    //@RubyLevelMethod(name="initialize")
    public RubyStringIO initialize(RubyValue arg) {
        this.value_ = (RubyString)arg.toRubyString().clone();
        return this;
    }

    //@RubyLevelMethod(name="string")
    public RubyString string() {
        return value_;
    }

    //@RubyLevelMethod(name="puts")
    public RubyString puts(RubyValue arg) {
        return value_.appendString(arg);
    }
    
    //@RubyLevelMethod(name="write")
    public RubyValue write(RubyValue arg) {
        return ObjectFactory.createFixnum(value_.appendString2(arg));
    }

    //@RubyLevelMethod(name="eof?")
    public RubyValue isEOF() {
        return ObjectFactory.createBoolean(nPos_>=value_.length());
    }
    
    //@RubyLevelMethod(name="read")
    public RubyValue read(RubyArray args) 
    {
    	switch ( args.size() )
    	{
    	case 0:
    		throw new RuntimeException("read with 0 parameters not implemented.");
    	case 1:
    		if ( nPos_>=value_.length() )
    			return RubyConstant.QNIL;
    		
    		int len = args.get(0).toInt();
    		String strRes = value_.getChars(nPos_,len);
    		nPos_ += strRes.length();
    		
    		return ObjectFactory.createString(strRes);
    	case 2:
    		throw new RuntimeException("read with 2 parameters not implemented.");
    	default:
    		throw new RubyException(RubyRuntime.ArgumentErrorClass, "in 'StringIO.read': wrong number of arguments : " + args.size() );
    	}
    }
    
}
