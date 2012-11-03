# Drupal Content Staging

This is a presentation about Drupal Content Staging using the Services
Client module, plus a pair of sample Drupal sites (code and database)
configured using the Services Client module for two-way content
sharing.

This presentation was first given at BADcamp 2012: 
http://2012.badcamp.net/program/sessions/content-staging-drupal-actually-works-finally

# Getting started

The demo is a pair of Drupal sites configured with two-way replication
of Page nodes (but not Article nodes) and users. 

The two Drupal site docroots are in the directories sc1 and sc2, with
their databases in sc1.sql and sc1.sql. sites/default/settings.php is
configured to use the database sc1 and sc2, respectively, as user root
with no password. Presumably you'll have to change that). 

Both sites have username admin, password admin.

The sites are configured to talk to each other via the Services module
via the endpoints http://localhost/sc1 and http://localhost/sc2. If
you deploy them at a different host name or directory, you need to
change the endpoints:

* Vist Structure > Services Client.
* Click Edit next to "Client" (sc1) or "Master" (sc2).
* Change the Endpoint field and press Save.

