Documentation
=============

| **Create Endpoints**
| Use the functions GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD on the server object to create a new Endpoint.

.. code-block:: cpp

    server.GET("/", handler);

| **Create Middleware**
| The function use() sets the given middleware for all endpoints.

.. code-block:: cpp

    server.use(handler);

| Creating middleware for one specific endpoint can be done by parsing it before the handler on endpoint creation.

.. code-block:: cpp

    server.GET("/", middlewareHandler, handler);

| **Send plain text, JSON, etc**
| Can be done by using the function data() in which you can specify everything yourself.

.. code-block:: cpp

    c.data("text/plain", Status::OK, "Hello World");

| But there are shortcuts using functions like string, json, status.

.. code-block:: cpp

    c.string("Hello World"); // Defaults to status 200 if not specified

| **Queries**
| Queries can be used with the query() function, which takes the query name and returns the content.
| For example, in the URL it can be used as /endpoint?queryName=queryBody

.. code-block:: cpp

    std::string message = c.query("message");

| **URL Parameters**
| If queries aren't enough, you can use URL parameters with the param() function.
| It takes the parameter name and gives back the content.  
| For it to work, you first need to specify the position of the parameter in the URL on endpoint creation using :

.. code-block:: cpp

    server.GET("/:test", handler);
    // LATER IN THE HANDLER
    std::string message = c.param("test");

| **Get Headers**
| Useful for Auth headers, etc. Can be used with the getHeader() function, which takes the header name and returns the content.

.. code-block:: cpp

    std::string message = c.getHeader("message");

| **Router Groups**
| Useful when you want to set middleware for all endpoints beneath another.
| For example, given the endpoint /test, it lets you set middleware for all endpoints beneath using g.use(middleware) or create new endpoints with g.GET("/example", handler), leading to the endpoint /test/example.

.. code-block:: cpp

    vespers::Router g = server.group("/test");

| **Redirects**
| Can be used when authorization fails and you want to send the user to the home page.

.. code-block:: cpp

    c.redirect("/");
    
| **Cookies**
| Useful when making a website that requires authentication. Then you can using setCookie() set a header which with every request gets sent from the browser,
| enabiling you to retrieve that data using the cookies() function.

.. code-block:: cpp

    c.setCookie("test", "1234");
    // After reconnecting cookie will be "1234"
    std::string cookie = c.cookies("test");