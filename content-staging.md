			   CONTENT STAGING
				 for
				DRUPAL

			     BADCAMP 2012

			     Barry Jaspan
			     Acquia, Inc.

# Architecture

* Content source site
  * Content creation and editing
  * Pushes to recipients when told to

* Content sink site
  * Receives new or updated content from source

* Content sinks can also be sources
  * Allows N-way distribution
  * Detects and prevents loops
  * Conflicts are possible!
    * Store all updates as revisions

# Implementation

* UUID
  * Uniquely identifies objects across all sites

* Services module
  * Basic API support: REST/XMLPRC, JSON/XML, authentication, ...
  * Exposes form-based APIs for nodes and other types

* Services Raw module
  * Exposes object-based APIs for nodes and other types

* Serivices UUID module
  * Exposes UUID APIs via Services

* Services Client, Services Client Connection modules
  * Manages connections to remote site REST servers

* Services Client Custom Conditions modules
  * Allow custom conditions to determine which remote connection(s)
    receive new and updated content.

# Use case: Editorial workflow

DIAGRAM: What Cooper and I drew on the whiteboard. Manager site
pushing to Staging, Production, identifying the various modules.

* Content Manager site
  * Primary content creation and editing site
  * Workflow, approval process, etc.
  * Distribution triggered by workflow

* Staging site
  * Receives updates from Content Manager
  * Same code as production site
  * DB and files updated regularly
  * Shows content in context: most viewed, etc.
  * Does not need Manager, Workflow modules

* Production site
  * Receives updates from Content Manager
  * Does not need Manager, Workflow modules

* Two-way production site
  * Allows editing content on live site
  * Pushes changes to Content Manager
  * Requires Manager, Workflow modules

# Use case: Multiple domains

* Content Manager site
  * Content creation and editing
  * Destination site defined by taxonomy
  * Distribution triggered by hook_node_save()

* domain-1.com, domain-2.com, ...
  * Receives updates from Content Manager
  * Does not need Manager, Workflow modules

# Use case: Hub-and-spoke federated content

DIAGRAM: CCI

* Multiple rim sites
  * Content creation and editing, often programmatically
  * Pushes changes to central hub site

* Central hub site
  * Mainly a read-only distribution center
  * Holds "master" copy of content
  * All updates stored as revisions
  * Distributes changes to rim sites

