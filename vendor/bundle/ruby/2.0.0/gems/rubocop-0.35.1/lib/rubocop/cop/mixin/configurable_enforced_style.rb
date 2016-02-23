# encoding: utf-8

module RuboCop
  module Cop
    # Handles `EnforcedStyle` configuration parameters.
    module ConfigurableEnforcedStyle
      def opposite_style_detected
        style_detected(alternative_style)
      end

      def correct_style_detected
        style_detected(style)
      end

      def unexpected_style_detected(unexpected)
        style_detected(unexpected)
      end

      def ambiguous_style_detected(*possibilities)
        style_detected(possibilities)
      end

      def style_detected(detected)
        # `detected` can be a single style, or an Array of possible styles
        # (if there is more than one which matches the observed code)

        return if no_acceptable_style?

        if detected.is_a?(Array)
          detected.map!(&:to_s)
        else
          detected = detected.to_s
        end

        if !detected_style # we haven't observed any specific style yet
          self.detected_style = detected
        elsif detected_style.is_a?(Array)
          self.detected_style &= [*detected]
        elsif detected.is_a?(Array)
          no_acceptable_style! unless detected.include?(detected_style)
        else
          no_acceptable_style! unless detected_style == detected
        end
      end

      def no_acceptable_style?
        config_to_allow_offenses['Enabled'] == false
      end

      def no_acceptable_style!
        self.config_to_allow_offenses = { 'Enabled' => false }
      end

      def detected_style
        config_to_allow_offenses[parameter_name]
      end

      def detected_style=(style)
        if style.nil?
          no_acceptable_style!
        elsif style.is_a?(Array)
          if style.empty?
            no_acceptable_style!
          elsif style.one?
            config_to_allow_offenses[parameter_name] = style[0]
          else
            config_to_allow_offenses[parameter_name] = style
          end
        else
          config_to_allow_offenses[parameter_name] = style
        end
      end

      alias_method :conflicting_styles_detected, :no_acceptable_style!
      alias_method :unrecognized_style_detected, :no_acceptable_style!

      def style
        s = cop_config[parameter_name]
        if cop_config['SupportedStyles'].include?(s)
          s.to_sym
        else
          fail "Unknown style #{s} selected!"
        end
      end

      def alternative_style
        a = cop_config['SupportedStyles'].map(&:to_sym)
        if a.size != 2
          fail 'alternative_style can only be used when there are exactly ' \
               '2 SupportedStyles'
        end
        style == a.first ? a.last : a.first
      end

      def parameter_name
        'EnforcedStyle'
      end
    end
  end
end
