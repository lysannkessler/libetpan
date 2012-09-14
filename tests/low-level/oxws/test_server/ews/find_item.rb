require 'rubygems'

module LibetpanTest
module OXWS
module Handlers

class FindItem
  def initialize(ews)
    @ews = ews
  end

  def handle(message_node)
    return @ews.nokogiri :find_item
  end
end

end
end
end
