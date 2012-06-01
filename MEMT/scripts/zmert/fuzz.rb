FUZZ_SLIDE_AMOUNT=11
class Fuzz
  def self.slide_amount
    FUZZ_SLIDE_AMOUNT
  end
  attr_reader :slide
  def initialize
    @mult = 1.1
    @slide = FUZZ_SLIDE_AMOUNT
  end
  def amount(iter)
    return 0.0 if iter >= @slide
    return (@slide - iter).to_f / @slide.to_f * @mult
  end
  def string_amount(iter)
    "score.fuzz.ratio = #{amount(iter)}\n"
  end
end

