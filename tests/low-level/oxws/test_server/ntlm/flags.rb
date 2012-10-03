module LibetpanTest
module OXWS

class NTLMFlags
  KEYS = %w[A B C r10 D E F G r9 H r8 J K L r7 M N O r6 P Q r5 R S r4 T r3 r2 r1 U V W]
  ALIASES = {
    'request_target' => 'C',
    'negotiate_ntlm' => 'H',
    'negotiate_oem_domain_supplied' => 'K',
    'negotiate_oem_workstation_supplied' => 'L',
    'negotiate_always_sign' => 'M',
    'negotiate_target_info' => 'S',
    'negotiate_version' => 'T'
  }

  def initialize(*bits_set)
    @hash = {}
    bits_set.each do |name|
      if KEYS.include? name
        @hash[name] = true
      elsif ALIASES.has_key? name
        @hash[ALIASES[name]] = true
      end
    end
  end

  def self.set?(hash, key)
    if KEYS.include? key
      hash[key] || false
    elsif ALIASES.has_key? key
      self.set? hash, ALIASES[key]
    else
      nil
    end
  end
  def set?(key)
    self.class.set? @hash, key
  end
  alias :[] :set?

  def self.set(hash, key, value = true)
    if KEYS.include? key
      hash[key] = value ? true : false
    elsif ALIASES.has_key? key
      self.set hash, ALIASES[key], value
    end
  end
  def set(key, value = true)
    self.class.set @hash, key, value
  end
  def []=(key, value)
    self.set key, value
  end

  def serialize
    [ KEYS.map{|k| self[k] ? "1" : "0"}.join ].pack('b'+KEYS.length.to_s)
  end

  def self.deserialize(integer)
    self.new Hash[KEYS.length.times.map {|i| [KEYS[i], integer[i]]}]
  end
end

end
end
