.PHONY: all mount run test clean

all:
	bundle install || exit 1
	cd judged && make || exit 1

mount:
	sudo mount -t tmpfs -o size=100M,noatime,nodev,nosuid koj-tmp tmp

run: mount
	ruby judger.rb

test: clean
	(ruby judger.rb &)
	cd test && ruby test.rb && sleep 3
	(cat log/judger.pid | xargs kill) && rm log/judger.pid

mq_reset:
	sudo rabbitmqctl stop_app
	sudo rabbitmqctl reset
	sudo rabbitmqctl start_app
	sudo rabbitmqctl list_queues

mq_list:
	sudo rabbitmqctl list_queues

clean:
	rm -f log/*
	rm -rf tmp/*-*
