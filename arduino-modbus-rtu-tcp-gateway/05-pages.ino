/* *******************************************************************
   Pages for Webserver

   sendPage()
   - sends the requested page (incl. 404 error and JSON document)
   - displays main page, renders title and left menu using <div> 
   - calls content functions depending on the number (i.e. URL) of the requested web page
   - also displays buttons for some of the pages
   - in order to save flash memory, some HTML closing tags are omitted, new lines in HTML code are also omitted

   contentInfo(), contentStatus(), contentIp(), contentTcp(), contentRtu()
   - render the content of the requested page

   contentWait()
   - renders the "please wait" message instead of the content, will be forwarded to home page after 5 seconds

   tagInputNumber(), tagLabelDiv(), tagButton(), tagDivClose(), tagSpan()
   - render snippets of repetitive HTML code for <input>, <label>, <div>, <button> and <span> tags

   stringPageName(), stringStats()
   - renders repetitive strings for menus, error counters

   jsonVal()
   - provide JSON value to a corresponding JSON key

   ***************************************************************** */

const byte WEB_OUT_BUFFER_SIZE = 64;  // size of web server write buffer (used by StreamLib)

void sendPage(EthernetClient &client, byte reqPage) {
  char webOutBuffer[WEB_OUT_BUFFER_SIZE];
  ChunkedPrint chunked(client, webOutBuffer, sizeof(webOutBuffer));  // the StreamLib object to replace client print
  if (reqPage == PAGE_ERROR) {
    chunked.print(F("HTTP/1.1 404 Not Found\r\n"
                    "\r\n"
                    "404 Not found"));
    chunked.end();
    return;
  } else if (reqPage == PAGE_DATA) {
    chunked.print(F("HTTP/1.1 200\r\n"
                    "Content-Type: application/json\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "\r\n"));
    chunked.begin();
    chunked.print(F("{"));
    for (byte i = 0; i < JSON_LAST; i++) {
      if (i) chunked.print(F(","));
      chunked.print(F("\""));
      chunked.print(i);
      chunked.print(F("\":\""));
      jsonVal(chunked, i);
      chunked.print(F("\""));
    }
    chunked.print(F("}"));
    chunked.end();
    return;
  }
  chunked.print(F("HTTP/1.1 200 OK\r\n"
                  // "Connection: close\r\n"
                  "Content-Type: text/html\r\n"
                  "Transfer-Encoding: chunked\r\n"
                  "\r\n"));
  chunked.begin();
  chunked.print(F("<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<meta"));
  if (reqPage == PAGE_WAIT) {  // redirect to new IP and web port
    chunked.print(F(" http-equiv=refresh content=5;url=http://"));
    chunked.print(IPAddress(localConfig.ip));
    chunked.print(F(":"));
    chunked.print(localConfig.webPort);
  }
  chunked.print(F(">"
                  "<title>Modbus RTU &rArr; Modbus TCP/UDP Gateway</title>"
                  "<style>"
                  "body,.m{padding:1px;margin:0;font-family:sans-serif}"  // class=m  - navigation menu (left)
                  "h1,h4{padding:10px}"                                   // h1 - main title of the page
                  "h1,.m,h4{background:#0067AC;margin:1px}"               // h4 - text in navigation menu and header of page content
                  ".m,.c{height:calc(100vh - 71px)}"                      // class=c - content of a page
                  ".m{min-width:20%}"                                     //
                  ".c{flex-grow:1;overflow-y:auto}"                       //
                  ".w,.r{display:flex}"                                   // class=w - wrapper (m + c)
                  "a,h1,h4{color:white;text-decoration:none}"             // a - items in left navigation menu
                  ".c h4{padding-left:30%;margin-bottom:20px}"            //
                  ".r{margin:4px}"                                        // class=r - row inside content
                  "label{width:30%;text-align:right;margin-right:2px}"    // label - left side (column) of a row
                  "input,button,select{margin-top:-2px}"                  // improve vertical allignment of input, button and select
                  ".s{text-align:right}"                                  // class=s - select input with numbers
                  ".s>option{direction:rtl}"                              //
                  ".i{text-align:center;width:3ch;color:black}"           // class=i - input for Modbus request (and Modbus response)
                  ".n{width:8ch}"                                         // class=n - input type=number
                  "</style>"
                  "</head>"
                  "<body"));
#ifdef ENABLE_DHCP
  chunked.print(F(" onload='dis(document.getElementById(&quot;box&quot;).checked)'>"
                  "<script>function dis(st) {var x = document.getElementsByClassName('ip');for (var i = 0; i < x.length; i++) {x[i].disabled = st}}</script"));
#endif /* ENABLE_DHCP */
  if (reqPage == PAGE_STATUS) {
    chunked.print(F("><script>"
                    "var a;"
                    "const renew=()=>{"
                    "fetch('d.json')"  // Call the fetch function passing the url of the API as a parameter
                    ".then(resp=>{return resp.json();a=0})"
                    ".then(jo=>{for(var i in jo){if(document.getElementById(i))document.getElementById(i).innerHTML=jo[i];}})"
                    ".catch(()=>{if(!a){alert('Connnection lost');a=1}})"
                    "};"
                    "setInterval(()=>renew(),"));
    chunked.print(FETCH_INTERVAL);
    chunked.print(F(");"
                    "</script"));
  }
  chunked.print(F(">"
                  "<h1>Modbus RTU &rArr; Modbus TCP/UDP Gateway</h1>"
                  "<div class=w>"
                  "<div class=m>"));

  // Left Menu
  for (byte i = 1; i <= PAGE_RTU; i++) {  // RTU Settings are the last item in the left menu
    chunked.print(F("<h4 "));
    if ((i) == reqPage) {
      chunked.print(F(" style=background-color:#FF6600"));
    }
    chunked.print(F("><a href="));
    chunked.print(i);
    chunked.print(F(".htm>"));
    stringPageName(chunked, i);
    chunked.print(F("</a></h4>"));
  }
  chunked.print(F("</div>"  // <div class=w>
                  "<div class=c>"
                  "<h4>"));
  stringPageName(chunked, reqPage);
  chunked.print(F("</h4>"
                  "<form method=post>"));

  //   PLACE FUNCTIONS PROVIDING CONTENT HERE

  switch (reqPage) {
    case PAGE_INFO:
      contentInfo(chunked);
      break;
    case PAGE_STATUS:
      contentStatus(chunked);
      break;
    case PAGE_IP:

#ifndef TEST_SOCKS
      contentIp(chunked);
#endif
      break;
    case PAGE_TCP:
      contentTcp(chunked);
      break;
    case PAGE_RTU:
      contentRtu(chunked);
      break;
    case PAGE_WAIT:
      contentWait(chunked);
      break;
    default:
      break;
  }

  if (reqPage == PAGE_IP || reqPage == PAGE_TCP || reqPage == PAGE_RTU) {
    chunked.print(F("<p><div class=r><label><input type=submit value='Save & Apply'></label><input type=reset value=Cancel></div>"));
  }
  chunked.print(F("</form>"));
  tagDivClose(chunked);  // close tags <div class=c> <div class=w>
  chunked.end();         // closing tags not required </body></html>
  // client.stop();
}


//        System Info
void contentInfo(ChunkedPrint &chunked) {
  tagLabelDiv(chunked, F("SW Version"));
  chunked.print(VERSION[0]);
  chunked.print(F("."));
  chunked.print(VERSION[1]);
  tagButton(chunked, F("Load Default Settings"), FACTORY);
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Microcontroller"));
  chunked.print(BOARD);
  tagButton(chunked, F("Reboot"), REBOOT);
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Ethernet Chip"));
  switch (W5100.getChip()) {
    case 51:
      chunked.print(F("W5100"));
      break;
    case 52:
      chunked.print(F("W5200"));
      break;
    case 55:
      chunked.print(F("W5500"));
      break;
    default:  // TODO: add W6100 once it is included in Ethernet library
      chunked.print(F("Unknown"));
      break;
  }
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("MAC Address"));
  byte macBuffer[6];
  W5100.getMACAddress(macBuffer);
  for (byte i = 0; i < 6; i++) {
    chunked.print(hex(macBuffer[i]));
    if (i < 5) chunked.print(F(":"));
  }
  tagButton(chunked, F("Generate New MAC"), MAC);
  tagDivClose(chunked);

#ifdef ENABLE_DHCP
  tagLabelDiv(chunked, F("Auto IP"));
  if (!extraConfig.enableDhcp) {
    chunked.print(F("DHCP disabled"));
  } else if (dhcpSuccess == true) {
    chunked.print(F("DHCP successful"));
  } else {
    chunked.print(F("DHCP failed, using fallback static IP"));
  }
  tagDivClose(chunked);
#endif /* ENABLE_DHCP */

  tagLabelDiv(chunked, F("IP Address"));
  chunked.print(IPAddress(Ethernet.localIP()));
  tagDivClose(chunked);
}

//        Modbus Status
void contentStatus(ChunkedPrint &chunked) {

#ifdef ENABLE_EXTRA_DIAG
  tagLabelDiv(chunked, F("Run Time"));
  tagSpan(chunked, JSON_DAYS);
  chunked.print(F(" days, "));
  tagSpan(chunked, JSON_HOURS);
  chunked.print(F(" hours, "));
  tagSpan(chunked, JSON_MINS);
  chunked.print(F(" mins, "));
  tagSpan(chunked, JSON_SECS);
  chunked.print(F(" secs"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("RTU Data"));
  tagSpan(chunked, JSON_RTU_TX);
  chunked.print(F(" Tx bytes / "));
  tagSpan(chunked, JSON_RTU_RX);
  chunked.print(F(" Rx bytes"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Ethernet Data"));
  tagSpan(chunked, JSON_ETH_TX);
  chunked.print(F(" Tx bytes / "));
  tagSpan(chunked, JSON_ETH_RX);
  chunked.print(F(" Rx bytes  (excl. WebUI)"));
  tagDivClose(chunked);
#endif /* ENABLE_EXTRA_DIAG */

#ifdef TEST_SOCKS
  tagSpan(chunked, JSON_SOCKETS);
#endif

  tagLabelDiv(chunked, F("Modbus RTU Request"));
  for (byte i = 0; i <= POST_REQ_LAST - POST_REQ; i++) {
    chunked.print(F("<input name="));
    chunked.print((POST_REQ + i), HEX);
    if (i == 0 || i == 1) {
      chunked.print(F(" required"));  // first byte (slave address) and second byte (function code) are required
    }
    chunked.print(F(" minlength=2 maxlength=2 class=i pattern='[a-fA-F&bsol;d]+' value='"));
    if (i < requestLen) {
      chunked.print(hex(request[i]));
    }
    chunked.print(F("'>"));
  }
  chunked.print(F("h (without CRC) <input type=submit value=Send>"));
  tagButton(chunked, F("Clear"), CLEAR_REQUEST);
  tagDivClose(chunked);
  chunked.print(F("</form><form method=post>"));
  tagLabelDiv(chunked, F("Modbus RTU Response"));
  tagSpan(chunked, JSON_RESPONSE);
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Requests Queue"));
  tagSpan(chunked, JSON_QUEUE_DATA);
  chunked.print(F(" / "));
  chunked.print(MAX_QUEUE_DATA);
  chunked.print(F(" bytes<br>"));
  tagSpan(chunked, JSON_QUEUE_REQUESTS);
  chunked.print(F(" / "));
  chunked.print(MAX_QUEUE_REQUESTS);
  chunked.print(F(" requests"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Modbus Statistics"));
  tagButton(chunked, F("Reset Stats"), RESET_STATS);
  for (byte i = 0; i < 4; i++) {  // only first four Modbus status counters are displayed (there is no counter for SLAVE_ERROR_0B_QUEUE)
    tagSpan(chunked, JSON_ERROR + i);
    stringStats(chunked, i);
  }
  tagSpan(chunked, JSON_ERROR_TCP);
  chunked.print(F(" Invalid TCP/UDP Request<br>"));
  tagSpan(chunked, JSON_ERROR_RTU);
  chunked.print(F(" Invalid RTU Response<br>"));
  tagSpan(chunked, JSON_ERROR_TIMEOUT);
  chunked.print(F(" Response Timeout"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Modbus Masters"));
  tagSpan(chunked, JSON_TCP_UDP_MASTERS);
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Modbus Slaves"));
  tagButton(chunked, F("Scan Slaves"), SCAN);
  tagSpan(chunked, JSON_SLAVES);
  tagDivClose(chunked);
}

//            IP Settings
void contentIp(ChunkedPrint &chunked) {

#ifdef ENABLE_DHCP
  tagLabelDiv(chunked, F("Auto IP"));
  chunked.print(F("<input type=hidden name="));
  chunked.print(POST_DHCP, HEX);
  chunked.print(F(" value=0>"
                  "<input type=checkbox id=box name="));
  chunked.print(POST_DHCP, HEX);
  chunked.print(F(" onclick=dis(this.checked) value=1"));
  if (extraConfig.enableDhcp) chunked.print(F(" checked"));
  chunked.print(F(">Enable DHCP"));
  tagDivClose(chunked);
#endif /* ENABLE_DHCP */

  for (byte j = 0; j < 3; j++) {
    switch (j) {
      case 0:
        tagLabelDiv(chunked, F("Static IP"));
        break;
      case 1:
        tagLabelDiv(chunked, F("Submask"));
        break;
      case 2:
        tagLabelDiv(chunked, F("Gateway"));
        break;
      default:
        break;
    }
    for (byte i = 0; i < 4; i++) {
      chunked.print(F("<input name="));
      chunked.print(POST_IP + i + (j * 4), HEX);
      chunked.print(F(" class='ip i' required maxlength=3 pattern='^(&bsol;d{1,2}|1&bsol;d&bsol;d|2[0-4]&bsol;d|25[0-5])$' value="));
      switch (j) {
        case 0:
          chunked.print(localConfig.ip[i]);
          break;
        case 1:
          chunked.print(localConfig.subnet[i]);
          break;
        case 2:
          chunked.print(localConfig.gateway[i]);
          break;
        default:
          break;
      }
      chunked.print(F(">"));
      if (i < 3) chunked.print(F("."));
    }
    tagDivClose(chunked);
  }
#ifdef ENABLE_DHCP
  tagLabelDiv(chunked, F("DNS Server"));
  for (byte i = 0; i < 4; i++) {
    chunked.print(F("<input name="));
    chunked.print(POST_DNS + i, HEX);
    chunked.print(F(" type=tel class=ip required maxlength=3 size=3 pattern='^(&bsol;d{1,2}|1&bsol;d&bsol;d|2[0-4]&bsol;d|25[0-5])$' value="));
    chunked.print(extraConfig.dns[i]);
    chunked.print(F(">"));
    if (i < 3) chunked.print(F("."));
  }
  tagDivClose(chunked);
#endif /* ENABLE_DHCP */
}

//            TCP/UDP Settings
void contentTcp(ChunkedPrint &chunked) {
  for (byte i = 0; i < 3; i++) {
    switch (i) {
      case 0:
        tagLabelDiv(chunked, F("Modbus TCP Port"));
        break;
      case 1:
        tagLabelDiv(chunked, F("Modbus UDP Port"));
        break;
      case 2:
        tagLabelDiv(chunked, F("WebUI Port"));
        break;
      default:
        break;
    }
    tagInputNumber(chunked);
    chunked.print(POST_TCP + i, HEX);
    chunked.print(F(" min=1 max=65535 value="));
    switch (i) {
      case 0:
        chunked.print(localConfig.tcpPort);
        break;
      case 1:
        chunked.print(localConfig.udpPort);
        break;
      case 2:
        chunked.print(localConfig.webPort);
        break;
      default:
        break;
    }
    chunked.print(F(">"));
    tagDivClose(chunked);
  }
  tagLabelDiv(chunked, F("Modbus Mode"));
  chunked.print(F("<select name="));
  chunked.print(POST_RTU_OVER, HEX);
  chunked.print(F(">"));
  for (byte i = 0; i < 2; i++) {
    chunked.print(F("<option value="));
    chunked.print(i);
    if (localConfig.enableRtuOverTcp == i) chunked.print(F(" selected"));
    chunked.print(F(">"));
    switch (i) {
      case 0:
        chunked.print(F("Modbus TCP/UDP"));
        break;
      case 1:
        chunked.print(F("Modbus RTU over TCP/UDP"));
        break;
      default:
        break;
    }
    chunked.print(F("</option>"));
  }
  chunked.print(F("</select>"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Modbus TCP Idle Timeout"));
  tagInputNumber(chunked);
  chunked.print(POST_TCP_TIMEOUT, HEX);
  chunked.print(F(" min=1 max=3600 value="));
  chunked.print(localConfig.tcpTimeout);
  chunked.print(F("> (1~3600) sec"));
  tagDivClose(chunked);
}

//            RTU Settings
void contentRtu(ChunkedPrint &chunked) {
  tagLabelDiv(chunked, F("Baud Rate"));
  chunked.print(F("<select class=s name="));
  chunked.print(POST_BAUD, HEX);
  chunked.print(F(">"));
  for (byte i = 0; i < (sizeof(BAUD_RATES) / 2); i++) {
    chunked.print(F("<option value="));
    chunked.print(BAUD_RATES[i]);
    if (localConfig.baud == BAUD_RATES[i]) chunked.print(F(" selected"));
    chunked.print(F(">"));
    chunked.print(BAUD_RATES[i]);
    chunked.print(F("00</option>"));
  }
  chunked.print(F("</select> bps"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Data Bits"));
  chunked.print(F("<select name="));
  chunked.print(POST_DATA, HEX);
  chunked.print(F(">"));
  for (byte i = 5; i <= 8; i++) {
    chunked.print(F("<option value="));
    chunked.print(i);
    if ((((localConfig.serialConfig & 0x06) >> 1) + 5) == i) chunked.print(F(" selected"));
    chunked.print(F(">"));
    chunked.print(i);
    chunked.print(F("</option>"));
  }
  chunked.print(F("</select> bit"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Parity"));
  chunked.print(F("<select name="));
  chunked.print(POST_PARITY, HEX);
  chunked.print(F(">"));
  for (byte i = 0; i <= 3; i++) {
    if (i == 1) continue;  // invalid value, skip and continue for loop
    chunked.print(F("<option value="));
    chunked.print(i);
    if (((localConfig.serialConfig & 0x30) >> 4) == i) chunked.print(F(" selected"));
    chunked.print(F(">"));
    switch (i) {
      case 0:
        chunked.print(F("None"));
        break;
      case 2:
        chunked.print(F("Even"));
        break;
      case 3:
        chunked.print(F("Odd"));
        break;
      default:
        break;
    }
    chunked.print(F("</option>"));
  }
  chunked.print(F("</select>"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Stop Bits"));
  chunked.print(F("<select name="));
  chunked.print(POST_STOP, HEX);
  chunked.print(F(">"));
  for (byte i = 1; i <= 2; i++) {
    chunked.print(F("<option value="));
    chunked.print(i);
    if ((((localConfig.serialConfig & 0x08) >> 3) + 1) == i) chunked.print(F(" selected"));
    chunked.print(F(">"));
    chunked.print(i);
    chunked.print(F("</option>"));
  }
  chunked.print(F("</select> bit"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Inter-frame Delay"));
  tagInputNumber(chunked);
  chunked.print(POST_FRAMEDELAY, HEX);
  chunked.print(F(" min="));
  byte minFrameDelay = byte(frameDelay() / 1000UL) + 1;
  chunked.print(minFrameDelay);
  chunked.print(F(" max=250 value="));
  chunked.print(localConfig.frameDelay);
  chunked.print(F("> ("));
  chunked.print(minFrameDelay);
  chunked.print(F("~250) ms"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Response Timeout"));
  tagInputNumber(chunked);
  chunked.print(POST_TIMEOUT, HEX);
  chunked.print(F(" min=50 max=5000 value="));
  chunked.print(localConfig.serialTimeout);
  chunked.print(F("> (50~5000) ms"));
  tagDivClose(chunked);
  tagLabelDiv(chunked, F("Attempts"));
  tagInputNumber(chunked);
  chunked.print(POST_ATTEMPTS, HEX);
  chunked.print(F(" min=1 max=5 value="));
  chunked.print(localConfig.serialAttempts);
  chunked.print(F("> (1~5)"));
  tagDivClose(chunked);
}


void contentWait(ChunkedPrint &chunked) {
  chunked.print(F("Reloading. Please wait..."));
}

// Functions providing snippets of repetitive HTML code
void tagInputNumber(ChunkedPrint &chunked) {
  chunked.print(F("<input class='s n' required type=number name="));
}

void tagLabelDiv(ChunkedPrint &chunked, const __FlashStringHelper *flashString) {
  chunked.print(F("<div class=r>"
                  "<label>"));
  chunked.print(flashString);
  chunked.print(F(":</label>"
                  "<div>"));
}

void tagButton(ChunkedPrint &chunked, const __FlashStringHelper *flashString, byte action) {
  chunked.print(F(" <button name="));
  chunked.print(POST_ACTION, HEX);
  chunked.print(F(" value="));
  chunked.print(action);
  chunked.print(F(">"));
  chunked.print(flashString);
  chunked.print(F("</button><br>"));
}

void tagDivClose(ChunkedPrint &chunked) {
  chunked.print(F("</div>"
                  "</div>"));  // <div class=r>
}

void tagSpan(ChunkedPrint &chunked, const byte JSONKEY) {
  chunked.print(F("<span id="));
  chunked.print(JSONKEY);
  chunked.print(F(">"));
  jsonVal(chunked, JSONKEY);
  chunked.print(F("</span>"));
}

// Menu item strings
void stringPageName(ChunkedPrint &chunked, byte item) {
  switch (item) {
    case PAGE_INFO:
      chunked.print(F("System Info"));
      break;
    case PAGE_STATUS:
      chunked.print(F("Modbus Status"));
      break;
    case PAGE_IP:
      chunked.print(F("IP Settings"));
      break;
    case PAGE_TCP:
      chunked.print(F("TCP/UDP Settings"));
      break;
    case PAGE_RTU:
      chunked.print(F("RTU Settings"));
      break;
    default:
      break;
  }
}

void stringStats(ChunkedPrint &chunked, const byte stat) {
  switch (stat) {
    case SLAVE_OK:
      chunked.print(F(" Slave Responded"));
      break;
    case SLAVE_ERROR_0X:
      chunked.print(F(" Slave Responded with Error (Codes 1~8)"));
      break;
    case SLAVE_ERROR_0A:
      chunked.print(F(" Gateway Overloaded (Code 10)"));
      break;
    case SLAVE_ERROR_0B:
    case SLAVE_ERROR_0B_QUEUE:
      chunked.print(F(" Slave Failed to Respond (Code 11)"));
      break;
    default:
      break;
  }
  chunked.print(F("<br>"));
}

void jsonVal(ChunkedPrint &chunked, const byte JSONKEY) {
  unsigned long temp = 0;
  switch (JSONKEY) {
    case JSON_TCP_UDP_MASTERS:
      {
        for (byte s = 0; s < maxSockNum; s++) {
          byte remoteIParray[4];
          W5100.readSnDIPR(s, remoteIParray);
          if (remoteIParray[0] != 0) {
            // if (uint32_t(remoteIParray) != 0) {
            if (W5100.readSnSR(s) == SnSR::UDP) {
              chunked.print(IPAddress(remoteIParray));
              chunked.print(F(" UDP<br>"));
            } else if (W5100.readSnSR(s) == SnSR::ESTABLISHED && W5100.readSnPORT(s) == localConfig.tcpPort) {
              chunked.print(IPAddress(remoteIParray));
              chunked.print(F(" TCP<br>"));
            }
          }
        }
      }
      return;
    case JSON_SLAVES:
      {
        for (byte k = 1; k < MAX_SLAVES; k++) {
          for (byte s = 0; s < SLAVE_ERROR_LAST; s++) {
            if (getSlaveStatus(k, s) == true || k == scanCounter) {
              chunked.print(hex(k));
              chunked.print(F("h"));
              if (k == scanCounter) {
                chunked.print(F(" Scanning...<br>"));
                break;
              }
              stringStats(chunked, s);
            }
          }
        }
      }
      return;
    case JSON_RESPONSE:
      {
        for (byte i = 0; i < MAX_RESPONSE_LEN; i++) {
          chunked.print(F("<input value='"));
          if (i < responseLen) {
            chunked.print(hex(response[i]));
          }
          chunked.print(F("' disabled class=i>"));
        }
        chunked.print(F("h"));
        if (responseLen > MAX_RESPONSE_LEN) {
          chunked.print(F(" +"));
          chunked.print(byte(responseLen - MAX_RESPONSE_LEN));
          chunked.print(F(" bytes"));
        }
      }
      return;
#ifdef TEST_SOCKS
    case JSON_SOCKETS:
      for (byte s = 0; s < maxSockNum; s++) {
        chunked.print(W5100.readSnPORT(s));
        chunked.print(F(" "));
        chunked.print(hex(W5100.readSnSR(s)));
        chunked.print(F(" "));
        chunked.print(millis() - lastSocketUse[s]);
        chunked.print(F("<br>"));
      }
      return;
#endif
    case JSON_ERROR ... JSON_ERROR_3:
      temp = errorCount[JSONKEY - JSON_ERROR];
      break;
    case JSON_ERROR_TCP:
      temp = errorTcpCount;
      break;
    case JSON_ERROR_RTU:
      temp = errorRtuCount;
      break;
    case JSON_ERROR_TIMEOUT:
      temp = errorTimeoutCount;
      break;
    case JSON_QUEUE_DATA:
      temp = (unsigned long)queueDataSize;
      queueDataSize = queueData.size();
      break;
    case JSON_QUEUE_REQUESTS:
      temp = (unsigned long)queueHeadersSize;
      queueHeadersSize = queueHeaders.size();
      break;
#ifdef ENABLE_EXTRA_DIAG
    case JSON_SECS:
      temp = (seconds) % 60L;
      break;
    case JSON_MINS:
      temp = (seconds / 60UL) % 60L;
      break;
    case JSON_HOURS:
      temp = (seconds / 3600UL) % 24L;
      break;
    case JSON_DAYS:
      temp = seconds / (3600UL * 24L);
      break;
    case JSON_RTU_TX:
      temp = serialTxCount;
      break;
    case JSON_RTU_RX:
      temp = serialRxCount;
      break;
    case JSON_ETH_TX:
      temp = ethTxCount;
      break;
    case JSON_ETH_RX:
      temp = ethRxCount;
      break;
#endif /* ENABLE_EXTRA_DIAG */
    default:
      break;
  }
  chunked.print(temp);
}
