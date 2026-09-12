#pragma once

#include "shared.hpp"
#include <boost/json.hpp>


class Controller {
public:

    virtual AsyncResponse GET(RequestContext& request);
    virtual AsyncResponse POST(RequestContext& request);
    virtual AsyncResponse PUT(RequestContext& request);
    virtual AsyncResponse DELETE(RequestContext& request);

    virtual ~Controller() = default;

};

