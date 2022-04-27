#include <cpprest/http_client.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;

#include <iostream>
using namespace std;

void display_json(
   json::value const & jvalue, 
   utility::string_t const & prefix)
{
   cout << prefix << jvalue.serialize() << endl;
}

pplx::task<http_response> make_task_request(
   http_client & client,
   method mtd,
   uri_builder builder,
   json::value const & jvalue)
{
   return (mtd == methods::GET || mtd == methods::HEAD) ?
      client.request(mtd, builder.to_string()) :
      client.request(mtd, builder.to_string(), jvalue);
}

void make_request(
   http_client & client,
   method mtd,
   uri_builder builder,
   json::value const & jvalue)
{
   make_task_request(client, mtd, builder, jvalue)
      .then([](http_response response)
      {
         if (response.status_code() == status_codes::OK)
         {
               return response.extract_json();
         }
         return pplx::task_from_result(json::value());
      })
      .then([](pplx::task<json::value> previousTask)
      {
         try
         {
               display_json(previousTask.get(), U("R: "));
         }
         catch (http_exception const & e)
         {
               cout << e.what() << endl;
         }
      })
      .wait();
}

int main()
{
   http_client client(U("http://localhost:9090"));
   uri_builder builder(U("/restdemo"));

   auto putvalue = json::value::object();
   putvalue[U("one")] = json::value::string(U("100"));
   putvalue[U("two")] = json::value::string(U("200"));

   cout << U("\nPUT (add values)\n");
   display_json(putvalue, U("S: "));
   make_request(client, methods::PUT, builder, putvalue);

   auto getvalue = json::value::array();
   getvalue[0] = json::value::string(U("one"));
   getvalue[1] = json::value::string(U("two"));
   getvalue[2] = json::value::string(U("three"));

   cout << U("\nPOST (get some values)\n");
   display_json(getvalue, U("S: "));
   make_request(client, methods::POST, builder, getvalue);

   auto delvalue = json::value::array();
   delvalue[0] = json::value::string(U("one"));

   cout << U("\nDELETE (delete values)\n");
   display_json(delvalue, U("S: "));
   make_request(client, methods::DEL, builder, delvalue);

   cout << U("\nPOST (get some values)\n");
   display_json(getvalue, U("S: "));
   make_request(client, methods::POST, builder, getvalue);

   auto nullvalue = json::value::null();

   cout << U("\nGET (get all values)\n");
   display_json(nullvalue, U("S: "));
   make_request(client, methods::GET, builder, nullvalue);

   return 0;
}