require 'rubygems'
require 'yaml'

module LibetpanTest
module OXWS

module Config
  def self.load_yaml(name)
    YAML.load_file File.expand_path(name + '.yaml', File.dirname(__FILE__))
  end

  def self.users
    @@users ||= self.load_yaml 'users'
  end
end

end
end
