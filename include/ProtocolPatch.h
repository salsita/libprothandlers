/****************************************************************************
 * libinetprotocolhook.h : Main include for libinetprotocolhook
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

namespace LibInetProtocolHook
{

//--------------------------------------------------------------------------
// patchProtokols:
//  Patches HTTP and HTTPS protocol handler.
HRESULT patchProtokols();

//--------------------------------------------------------------------------
// unpatchProtokols:
//  Unpatches HTTP and HTTPS protocol handler.
HRESULT unpatchProtokols();

//--------------------------------------------------------------------------
// setBrowserForRequest:
//  Sets the current browser for the upcoming request in the current thread.
//  This is meant to be called by OnBeforeRequest2 browser event handler
//  to make the browser available to the protocol implementation.
//  In StartEx the browser might not yet be available, so we need this
//  function to pass the browser.
HRESULT setBrowserForRequest(LPDISPATCH aDispatch, VARIANT *aVtUrl);

} // namespace LibInetProtocolHook
