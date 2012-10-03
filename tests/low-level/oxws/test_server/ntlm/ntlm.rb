require 'rubygems'
require 'rack/auth/abstract/handler'
require 'rack/auth/abstract/request'

require 'base64'

require File.expand_path('flags', File.dirname(__FILE__))


# Override Thin connection method to uniquely identify the connection.
# See LibetpanTest::OXWS::NTLM::Private's @@authorized_connections.
module Thin
  class Connection
    @@original_process = instance_method(:process)
    def process
      @request.env['thin.connection_id'] = self.object_id
      @@original_process.bind(self).call
    end
  end
end


module LibetpanTest
module OXWS

class NTLM < Rack::Auth::AbstractHandler

  attr_accessor :target_name

  def initialize(app, target_name, &authenticator)
    super(app, target_name, &authenticator)
    @target_name = target_name
    @authenticated_connections = {}
  end

  def call(env)
    # check if connection has already been authenticated
    unless @authenticated_connections.include? env['thin.connection_id']
      auth = NTLM::Request.new env
      # check Authorization header
      return unauthorized 'NTLM' unless auth.provided?
      return bad_request unless auth.scheme == :ntlm && auth.signature == "NTLMSSP\x00"

      # dispatch message type
      case auth.message_type
      when :negotiate
        return unauthorized 'NTLM ' + compute_challenge(auth)
      when :authenticate
        return unauthorized nil unless valid_authentication?(auth)
        # success: mark connection as authenticated
        @authenticated_connections[env['thin.connection_id']] = auth.username
      else
        return bad_request
      end
    end

    # remote end is authenticated
    env['REMOTE_USER'] = @authenticated_connections[env['thin.connection_id']]
    @app.call env
  end

  private

  # constants for challenge and for expected challenge responses for password 'shegalkin'
  SERVER_CHALLENGE = "\\\xE3k\xAA\x9CUQ\xDB" # Random.new.bytes(8)
  LM_CHALLENGE_RESPONSE = "\xC9\xA4&\xAA=\x863\x9C9z\xEA\x15o\x96\x15\x91\x8C\xEELR\xE6\x97\x7FM"
  NT_CHALLENGE_RESPONSE = "6\x05\x13\x1E_\xE0=\xEF~\x16\xB6\xFA\x1A\x8B\x03\xABW\xD6\xD5|\xE8)\x9E\xB4"

  def compute_challenge(auth)
    challenge_flags = NTLMFlags.new 'request_target', 'negotiate_ntlm', 'negotiate_always_sign'
    msg = { 'signature' => "NTLMSSP\x00", 'message_type' => 2, 'target_name' => target_name,
      'target_name_len' => target_name.length, 'target_name_max_len' => target_name.length, 'target_name_buffer_offset' => 48,
      'negotiate_flags' => challenge_flags.serialize, 'server_challenge' => SERVER_CHALLENGE,
      'reserved' => 0, 'target_info_fields' => 0 }
    msg_fields_order = %w[signature message_type
      target_name_len target_name_max_len target_name_buffer_offset
      negotiate_flags server_challenge reserved target_info_fields
      target_name]
    msg_bin = msg_fields_order.map { |key| msg[key] }.pack 'a8 L< S<S<L< b32 a8 Q< Q< a'+target_name.length.to_s
    Base64.encode64 msg_bin
  end

  def valid_authentication? auth
    # challenge responses must be as expected,
    # user provided authenticator must be successful
    auth.lm_challenge_response == LM_CHALLENGE_RESPONSE &&
    auth.nt_challenge_response == NT_CHALLENGE_RESPONSE &&
    @authenticator.call(*auth.credentials)
  end


  class Request < Rack::Auth::AbstractRequest
    def initialize(*args)
      @fields = {}
      super
    end

    def signature
      decode_head unless fields.include? 'signature'
      fields['signature']
    end
    def message_type
      decode_head unless fields.include? 'message_type'
      case fields['message_type']
      when 1 then :negotiate
      when 2 then :challenge
      when 3 then :authenticate
      else nil
      end
    end

    def credentials
      if message_type == :authenticate
        decode_authentication unless fields.include? 'user_name'
        @credentials ||= [
          fields['user_name'], fields['domain_name'],
          fields['lm_challenge_response'], fields['nt_challenge_response'] ]
      else
        nil
      end
    end
    def user_name; credentials[0] if credentials; end
    def domain_name; credentials[1] if credentials; end
    def lm_challenge_response; credentials[2] if credentials; end
    def nt_challenge_response; credentials[3] if credentials; end
    alias :username :user_name

    private

    attr_reader :fields

    def msg_bin
      @msg_bin ||= Base64.decode64 params
    end

    def decode_head
      decode_fields 'a8 L<', %w[signature message_type]
    end

    def decode_authentication
      # parse fixed fields (including flags)
      format = 'x12 S<x2L< S<x2L< S<x2L< S<x2L< x16 L<'
      keys = %w[lm_challenge_response_len lm_challenge_response_buffer_offset
        nt_challenge_response_len nt_challenge_response_buffer_offset
        domain_name_len domain_name_buffer_offset
        user_name_len user_name_buffer_offset
        negotiate_flags]
      decode_fields format, keys
      fields['negotiate_flags'] = NTLMFlags.deserialize fields['negotiate_flags']

      # parse remaining fields (based on flags)
      format = 'x64'
      keys = []
      %w[lm_challenge_response nt_challenge_response domain_name user_name].each do |key|
        format << ' @' + fields[key+'_buffer_offset'].to_s + 'a' + fields[key+'_len'].to_s
        keys << key
      end
      decode_fields format, keys
    end

    def decode_fields(format, keys)
      values = msg_bin.unpack format
      fields.merge! Hash[ keys.length.times.map { |i| [keys[i], values[i]] } ]
    end
  end

end

end
end
