require 'rubygems'
require 'sinatra/base'
require 'nokogiri'
require 'yaml'

module LibetpanTest
module OXWS

class Autodiscover < Sinatra::Base
  def self.users
    @@users ||= YAML.load_file File.expand_path('users.yaml', File.dirname(__FILE__))
  end

  set :nokogiri, :views => File.dirname(__FILE__)

  post '/autodiscover/autodiscover.xml', :provides => :xml do
    request_document = Nokogiri::XML.parse request.body
    email_address = request_document.xpath('/xmlns:Autodiscover/xmlns:Request/xmlns:EMailAddress').text

    user_info = self.class.users.find { |user| user['email'] == email_address }
    if user_info
      return nokogiri :success, :locals => {
        :user_info => user_info,
        :host => request.host,
        :host_with_port => request.host_with_port
      }
    else
      return nokogiri :error
    end
  end
end

end
end
