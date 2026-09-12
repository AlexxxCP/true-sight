#include "http/controller.hpp"
#include "http/default_responses.hpp"
#include "shared.hpp"

AsyncResponse Controller::GET(RequestContext& context)
{
    co_return responses::method_not_allowed(context);
}

AsyncResponse Controller::POST(RequestContext& context)
{
    co_return responses::method_not_allowed(context);
}

AsyncResponse Controller::PUT(RequestContext& context)
{
    co_return responses::method_not_allowed(context);
}

AsyncResponse Controller::DELETE(RequestContext& context)
{
    co_return responses::method_not_allowed(context);
}
