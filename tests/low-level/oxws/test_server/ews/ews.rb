require 'rubygems'
require 'sinatra/base'
require 'nokogiri'

require File.expand_path('find_item', File.dirname(__FILE__))

module LibetpanTest
module OXWS

class EWS < Sinatra::Base
  set :nokogiri, :views => File.dirname(__FILE__)

  get '/EWS/Exchange.asmx' do
    redirect '/EWS/Services.wsdl'
  end

  get '/EWS/Services.wsdl', :provides => :xml do
    filename = File.expand_path('services.wsdl', File.dirname(__FILE__))
    send_file filename, :disposition => 'inline'
  end

  post '/EWS/Exchange.asmx', :provides => :xml do
    begin
      request_document = Nokogiri::XML.parse request.body
      body = request_document.xpath('/soap:Envelope/soap:Body', 'soap' => 'http://schemas.xmlsoap.org/soap/envelope/')
      return error(:type => :fault_soap_ns, :string => 'Root element or SOAP body is missing.') if body.empty?
      handler = nil
      if body.children.count > 0
        message = body.children.first
        if message.namespace.href == 'http://schemas.microsoft.com/exchange/services/2006/messages'
          handler = self.handler(message)
        end
      end
      return error('Unable to handle request without a valid action parameter. Please supply a valid soap action.') if handler.nil?
    rescue Exception => e
      return error(:type => :fault_soap_ns, :string => e.to_s)
    end

    return handler.handle(message)
  end

  def handler(message_node)
    return case message_node.name
    when 'FindItem' then Handlers::FindItem.new(self)
    else nil
    end
  end

  def error(options = {})
    options = { :string => options } if options.is_a? String
    locals = { :detail => options[:detail] }
    type = options[:type] || :fault
    case type
    when :fault
      locals[:code] = options[:code] || 'soap:Client'
      locals[:string] = options[:string]
    when :fault_soap_ns
      locals[:code] = options[:code] || 'soap:Receiver'
      locals[:reason] = "Server was unable to process request. ---> #{options[:string]}"
    else
      raise "Error returning error response. Unsupported type: #{options[:type]}"
    end

    status 500
    nokogiri type, :locals => locals
  end
end

end
end
