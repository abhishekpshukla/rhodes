require File.dirname(File.join(__rhoGetCurrentDir(), __FILE__)) + '/../../spec_helper'
require File.dirname(File.join(__rhoGetCurrentDir(), __FILE__)) + '/fixtures/classes'

describe "Kernel#fork" do
  it "is a private method" do
    Kernel.should have_private_instance_method(:fork)
  end
end

describe "Kernel.fork" do
  it "needs to be reviewed for spec completeness"
end
