require File.dirname(File.join(__rhoGetCurrentDir(), __FILE__)) + '/../../spec_helper'

describe "TrueClass#^" do
  it "returns true if other is nil or false, otherwise false" do
    (true ^ true).should == false
    (true ^ false).should == true
    (true ^ nil).should == true
    (true ^ "").should == false
    (true ^ mock('x')).should == false
  end
end
