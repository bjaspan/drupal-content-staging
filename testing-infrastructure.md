




			TESTING INFRASTRUCTURE
				 for
			     DRUPAL SITES
				   
			     BADcamp 2012

			     Barry Jaspan
			     Acquia, Inc.



















# Intro to Acquia Cloud

* PaaS for PHP apps
  * LAMP stack
  * Multiple environments: Dev, Stage, Prod, ...
  * Provides a Continuous Integration environment for your app
  * Special sauce for Drupal

* Impressive Numbers(tm), ca. September 2012
  * 9 B hits per month
  * 142 TB data xfer/month
  * 3000+ EC2 instances

* Release every 1.6 days on average
  * Not just simple HTML/CSS/JS or database schema changes
  * Releases alter the infrastructure under thousands of web apps
    that we DO NOT CONTROL

* Our customers REALLY hate downtime

















# Intro to software development methodologies

* Waterfall (booo)
  * Kinda like pre-Dev Ops

* Agile (yay)
  * XP, Scrum, Kanban, ...
  * 'cause its better, usually

* 10 principles of Continuous Integration
  * Maintain a code repository
  * Automate the build
  * Make the build self-testing
  * Everyone commits to the baseline every day
  * Every commit (to baseline) should be built
  * Keep the build fast
  * Test in a clone of the production environment
  * Make it easy to get the latest deliverables
  * Everyone can see the results of the latest build
  * Automate deployment






















# Server configuration is software

* Deserves the same best practices as app development

* CI principles for today's talk: Testing!
  * Make the build self-testing
  * Test in a clone of the production environment
  * Keep the build fast

* "If it isn't tested, it doesn't work."

* For other CI principles, tune in next year...





















# Unit vs. System tests

* Unit tests isolate individual program modules
  * Injection mocks out external systems
  * Problem: You can't mock out the OS and get accurate results
  * "puppetd --noop" doesn't tell the whole story

* System tests are end-to-end
  * Apply code changes to real, running servers
  * Exercise the infrastructure as the app(s) will
  * Problem: Reality is very messy!
    * Race conditions and retries
    * EC2 launch failure
    * DNS vendor API scheduled maintenance
    * Cosmic rays

* For infrastructure, system tests are essential

* ACQUIA CLOUD SYSTEM TESTS ARE THE HARDEST THING I'VE EVER DONE
  * But we would be totally dead without them




















# Server build strategy

* Always build from a reference base
  * e.g. "Ubuntu 12.04 Server 64-bit us-east-1 AMI"
  * No incrementally evolved images
  * Puppet makes this natural

* Puppet can take a while
  * Makes tests and MTTR slow

* More on this later...





















# Basic build tests

* Launch VMs, run puppet
  * Replicate a functional production environment
  * Isolated from production

* Scan syslog for errors

* Test config files, daemons, users, cron jobs...





















# Functionally test the moving parts

* Backup and restore
* Message queues
* Worker auto-scaling
* ELB health check and recovery
* Load balancing with up and down workers
* Database failover
* Monitoring
* Alerting
* Self healing





















# Security-compliance compliant testing

* HIPPA, PCI, FISMA, etc. impose operational requirements

* Tests must comply with the same requirements
  * No special back-doors
  * "Test in a clone of production"

* Ex: Acquia Cloud requires two-factor auth and non-root logins,
  even for ops
  * WiKiD two-factor auth: token+PIN, SSH key/password
    * Computer as the "token"
    * CLI login via "expect"
  * ssh testadmin@server 'sudo bash -H -c "do_something"'





















# Application test

* Install and verify application(s)

* Force the app to exercise the infrastructure
  * Write to database, message queues, etc.
  * Verify success on the back end

* Force app to operate under degraded infrastructure conditions
  * Multiple web nodes
  * Database failover
  * ...





















# Reboot tests

* Reboot all test servers

* Re-run build tests
  * Filesystems mounted?
  * Services restarted?

* Re-run functional and application tests





















# Relaunch tests

* Relaunch all test servers from base image
  * Simulates EC2 instance loss and recovery
  * Persistent data retained?
  * Server rejoins services? (e.g. MySQL replication restarts)
  * Unexpected issues, ex: tmpfs

* Re-run build, functional, and application tests





















# Upgrade tests

* So far we've tested *new* servers
  * Use case: Your service is growing.

* Also need to test upgrading existing servers
  * Use case: You add a feature.

* The Upgrade Test Dance
  * Launch servers in test environment on current production code
  * Run build, functional, and application tests from production code
  * Upgrade servers to latest development code
  * Run build, functional, and application tests from development code





















# Upgrade release process

* Puppet cannot orchestrate all upgrades
  * Rolling upgrade across HA clusters
  * Server type upgrade order requirements
  * Post-release tasks
    * ex: Uninstall package X once all servers are upgraded

* We haven't automated the complete dependency matrix yet
  * Some tasks still performed by hand by ops: pdsh FTW!
  * Engineers are required to document and test release 
    procedure pre-commit





















# Server builds: Continuous Bundling

* Remember: Always build from a reference base

* Building servers from scratch can be slow

* Automate new image builds from development branch
  * Speeds intra-day tests, reduces MTTR

* You will hit unexpected bumps in the road
  * ex: MySQL server, EBS, init.d





















# Continuous Bundling image tests

* Create development images nightly

* Create per-branch images at release

* Test base and pre-built images

* Test upgrade from per-branch to development images

* We are just starting this process
  * Check back next year. :-)





















# Cleaning up

* Automate teardown
  * Keep a database of allocated/running resources
    * EC2 tags might be sufficient

* Audit for leak$
  * Or you will be $orry

* Separate cloud accounts for dev, test, and prod
  * Allows risk-free mass killing of non-production resources

    





















# Questions?

* Barry Jaspan
  barry.jaspan@acquia.com
  @bjaspan

* We're hiring
  * Boston
  * Portland
  * Europe!
  * Australia!!!
















