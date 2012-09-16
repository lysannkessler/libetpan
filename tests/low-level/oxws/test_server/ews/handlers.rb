require File.expand_path('find_item', File.dirname(__FILE__))
require File.expand_path('create_item', File.dirname(__FILE__))

module LibetpanTest
module OXWS

module Handlers
  def self.handler(ews, message_node)
    return case message_node.name
    when 'FindItem' then FindItem.new(ews)
    when 'CreateItem' then CreateItem.new(ews)
    else nil
    end
  end
end

end
end
