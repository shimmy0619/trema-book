Before('@sudo') do
  fail 'sudo authentication failed' unless system 'sudo -v'
  @aruba_timeout_seconds = 15
end

After('@sudo') do
  run 'trema killall'
  sleep 10
end

Before('@rest_api') do
  fail 'sudo authentication failed' unless system 'sudo -v'
end

After('@rest_api') do
  run 'trema killall'
  sleep 3
end

After('@rack') do
  in_current_dir do
    rack_pid = IO.read('rack.pid').chomp
    run "kill #{rack_pid}"
  end
end