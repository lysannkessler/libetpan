require File.expand_path(File.join('..', 'config', 'config'), File.dirname(__FILE__))

module LibetpanTest
module OXWS
module Handlers

class CreateItem
  def initialize(ews)
    @ews = ews
  end

  def handle(message_node)
    items = [
      { 'class_id' => 'message',
        'ItemId' => {
          'Id' => 'AAAhAEx5c2Fubi5LZXNzbGVyQGhwaS51bmktcG90c2RhbS5kZQBGAAAAAAAOQP8UchHYRYW2kLdtztl9BwDi0v2At9wNQ4mCHb9pSDgCAAAAe6dBAACac5yph9kYRpDIsFsCUOP+ACTMRrcJAAA=',
          'ChangeKey' => 'CQAAABYAAACac5yph9kYRpDIsFsCUOP+ACTMRrsJ'
        }
      }
    ]
    @ews.render_response :create_item, :locals => { :items => items }
  end
end

end
end
end
