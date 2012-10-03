require 'rubygems'
require 'sinatra/base'

require File.expand_path(File.join('config', 'config'), File.dirname(__FILE__))
require File.expand_path(File.join('ntlm', 'ntlm'), File.dirname(__FILE__))
require File.expand_path(File.join('ews', 'ews'), File.dirname(__FILE__))
require File.expand_path(File.join('autodiscover', 'autodiscover'), File.dirname(__FILE__))

module LibetpanTest
module OXWS

class WebApp < Sinatra::Base
  use NTLM, Config.domain do |user_name, domain_name|
    domain_name.downcase == Config.domain.downcase &&
    Config.users.any? { |user| user['user'] == user_name }
  end

  use EWS
  use Autodiscover

  enable :logging
end

end
end
