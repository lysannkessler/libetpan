require 'rubygems'
require 'sinatra/base'
require 'sinatra/partial'
require 'nokogiri'

require File.expand_path('handlers', File.dirname(__FILE__))

module LibetpanTest
module OXWS

class EWS < Sinatra::Base
  register Sinatra::Partial
  set :partial_template_engine, :nokogiri
  enable :partial_underscores

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
      # find SOAP envelope and body
      request_document = Nokogiri::XML.parse request.body
      body = request_document.xpath('/soap:Envelope/soap:Body', 'soap' => 'http://schemas.xmlsoap.org/soap/envelope/')
      return error(:type => :fault_soap_ns, :string => 'Root element or SOAP body is missing.') if body.empty?
      # find SOAP action and determine handler
      handler = nil
      message = body.xpath('./*') # this gets rid of text and comments
      if message.count > 0
        message = message.first
        if message.namespace && message.namespace.href == 'http://schemas.microsoft.com/exchange/services/2006/messages'
          handler = Handlers.handler(self, message)
        end
      end
      return error('Unable to handle request without a valid action parameter. Please supply a valid soap action.') if handler.nil?
    rescue Exception => e
      return error(:type => :fault_soap_ns, :string => e.to_s)
    end

    return handler.handle(message)
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

  def render_response(template, options)
    template_camel_case = template.to_s.split('_').map{|e| e.capitalize}.join
    locals = { :response_name => template_camel_case + 'Response',
               :response_message_name => template_camel_case + 'ResponseMessage',
               :template_name => template,
               :template_locals => options[:locals] }
    nokogiri :soap_response, :locals => locals
  end
end

end
end
