guard 'rake', task: 'index.html' do
  watch(/.+\.adoc$/)
end

guard :rubocop do
  watch(/.+\.rb$/)
  watch(%r{(?:.+/)?\.rubocop\.yml$}) { |m| File.dirname(m[0]) }
end
