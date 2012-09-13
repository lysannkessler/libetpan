require 'rubygems'
require 'sinatra/base'

require File.expand_path(File.join('ews', 'ews'), File.dirname(__FILE__))
require File.expand_path(File.join('autodiscover', 'autodiscover'), File.dirname(__FILE__))

module LibetpanTest
module OXWS

class WebApp < Sinatra::Base
  use EWS
  use Autodiscover
end

end
end
