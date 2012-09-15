require File.expand_path(File.join('..', 'config', 'config'), File.dirname(__FILE__))

module LibetpanTest
module OXWS
module Handlers

class FindItem
  def initialize(ews)
    @ews = ews
  end

  def handle(message_node)
    # TODO handle missing node, attributes, and wrong attribute types, values out of range
    indexed_paged_item_view = message_node.xpath('./m:IndexedPageItemView', 'xmlns:m' => 'http://schemas.microsoft.com/exchange/services/2006/messages').first
    base_point = indexed_paged_item_view['BasePoint']
    reverse_search = base_point == 'End'
    offset = indexed_paged_item_view['Offset'].to_i
    max_entries_returned = indexed_paged_item_view['MaxEntriesReturned'].to_i

    all_items = Config.items['inbox']
    if reverse_search
      items_end = all_items.count - 1 - offset
      items_end = 0 if items_end < 0
      items_start = items_end - max_entries_returned
      items_start = 0 if items_start < 0
    else
      items_start = offset
      items_end = items_start - 1 + max_entries_returned
    end
    items = all_items[items_start..items_end] || []
    items.reverse! if reverse_search

    @ews.nokogiri :find_item, :locals => {
      :indexed_paging_offset => offset,
      :total_items_in_view => all_items.count,
      :includes_last_item_in_range => items_end >= all_items.count - 1,
      :items => items
    }
  end
end

end
end
end
