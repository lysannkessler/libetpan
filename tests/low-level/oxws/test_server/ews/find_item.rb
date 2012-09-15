require File.expand_path(File.join('..', 'config', 'config'), File.dirname(__FILE__))

module LibetpanTest
module OXWS
module Handlers

class FindItem
  def initialize(ews)
    @ews = ews
  end

  def handle(message_node)
    @ews.nokogiri :find_item, :locals => { :items => Config.items['inbox'] }
  end
end

end
end
end
