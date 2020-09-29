# puppet
DSL with similar syntax to Javascript for use with WebDriver.

# External Dependencies
Puppet has the following libraries as external dependencies.

* MPIR
* libcurl

vcpkg and MSBuild are used to resolve these at the moment.

# TODO

* Actually learn build systems or introduce separate platform
  folders so that build process goes smoothly

* Use processes and netstat-like utilities on different platforms
  to create drivers and identify what port drivers are listening on.

* Try to talk to drivers using libcurl

* Actually make the lexer (Work on above points before this).


