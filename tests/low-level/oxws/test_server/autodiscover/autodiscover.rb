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

  @@route = '/autodiscover/autodiscover.xml'

  get @@route, :provides => :xml do
    return error_invalid_request
  end

  post @@route, :provides => :xml do
    begin
      request_document = Nokogiri::XML.parse request.body
      email_address_node = request_document.xpath('/xmlns:Autodiscover/xmlns:Request/xmlns:EMailAddress')
      raise 'Missing EMailAddress' if email_address_node.empty?
      email_address = email_address_node.text
    rescue Exception => e
      return error_invalid_request
    end

    user_info = self.class.users.find { |user| user['email'] == email_address }
    if user_info
      nokogiri :success, :locals => {
        :user_info => user_info,
        :host => request.host,
        :host_with_port => request.host_with_port
      }
    else
      error(500, 'The e-mail address cannot be found.')
    end
  end

  def error_invalid_request
    error(600, 'Invalid Request')
  end

  def error(code, message)
    nokogiri :error, :locals => { :error_code => code, :message => message }
  end
end

end
end
