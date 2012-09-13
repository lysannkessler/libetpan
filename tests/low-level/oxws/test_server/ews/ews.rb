require 'rubygems'
require 'sinatra/base'

module LibetpanTest
module OXWS

class EWS < Sinatra::Base
  get '/EWS/Exchange.asmx' do
    redirect '/EWS/Services.wsdl'
  end

  get '/EWS/Services.wsdl', :provides => :xml do
    filename = File.expand_path('services.wsdl', File.dirname(__FILE__))
    send_file filename, :disposition => 'inline'
  end
end

end
end
