/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

/* Setting a CR or LF in a string option is rejected, so the value cannot
   inject an extra line into the request or command it ends up in. */

#include "first.h"

static CURLcode test_lib1930(const char *URL)
{
  CURL *curl = NULL;
  CURLcode result = CURLE_OK;
  int fails = 0;
  size_t i;

  /* options whose value becomes (part of) a request line, header or command */
  static const CURLoption stropt[] = {
    CURLOPT_URL, CURLOPT_USERAGENT, CURLOPT_REFERER,
    CURLOPT_RANGE, CURLOPT_USERNAME, CURLOPT_PASSWORD
  };
  static const char * const injected[] = {
    "value\r\nInjected: 1",  /* CRLF */
    "value\rInjected",       /* bare CR */
    "value\nInjected"        /* bare LF */
  };

  (void)URL;

  global_init(CURL_GLOBAL_ALL);
  easy_init(curl);

  for(i = 0; i < sizeof(stropt) / sizeof(stropt[0]); i++) {
    size_t j;
    for(j = 0; j < sizeof(injected) / sizeof(injected[0]); j++) {
      if(curl_easy_setopt(curl, stropt[i], injected[j]) !=
         CURLE_BAD_FUNCTION_ARGUMENT) {
        curl_mprintf("option %d accepted a line break (case %d)\n",
                     (int)stropt[i], (int)j);
        fails++;
      }
    }
    /* a clean value still works */
    if(curl_easy_setopt(curl, stropt[i], "clean-value") != CURLE_OK) {
      curl_mprintf("option %d rejected a clean value\n", (int)stropt[i]);
      fails++;
    }
    /* a tab is not a line break and stays accepted */
    if(curl_easy_setopt(curl, stropt[i], "one\ttwo") != CURLE_OK) {
      curl_mprintf("option %d rejected a tab\n", (int)stropt[i]);
      fails++;
    }
  }

  /* a POST body may legitimately hold CR/LF, so it must not be rejected */
  if(curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, "a=1\r\nb=2") ==
     CURLE_BAD_FUNCTION_ARGUMENT) {
    curl_mprintf("POST body wrongly rejected for a line break\n");
    fails++;
  }

  curl_mprintf("%d failures\n", fails);
  result = fails ? TEST_ERR_FAILURE : CURLE_OK;

test_cleanup:
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return result;
}
