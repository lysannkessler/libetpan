require 'rubygems'
require 'yaml'
require 'date'

module LibetpanTest
module OXWS

module Config
  def self.load_yaml(name)
    YAML.load_file File.expand_path(name + '.yaml', File.dirname(__FILE__))
  end

  def self.users
    @@users ||= self.load_yaml 'users'
  end

  @@items_per_folder = nil
  def self.items
    if @@items_per_folder.nil?
      @@items_per_folder = self.load_yaml 'items'
      @@items_per_folder.each do |name, items|
        items.each { |item| self.convert_item item }
      end
    end
    @@items_per_folder
  end

  private

  def self.convert_item(item)
    item['DateTimeSent'] = self.convert_datetime(item['DateTimeSent'])
    item['DateTimeCreated'] = self.convert_datetime(item['DateTimeCreated'])
    return item
  end

  def self.convert_datetime(string)
    return DateTime.parse(string)
  end
end

end
end
