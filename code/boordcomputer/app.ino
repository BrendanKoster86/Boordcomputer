bool app_sturen(String bericht) {
  
  bool verzonden = false;
  
  if (app_scherm_actief) {
    if (scherm_helderheid == 100) {
      digitalWrite(led, HIGH);
    } else {
      analogWrite(led, 255 * (scherm_helderheid / 100));
    }
    beeld_melding("Even geduld", "Bezig met het versturen", "Bericht versturen");
  }
  
  wifi_check();
    // delay(500);
  if (wifi_connectie_modus != 0) {
    wifi_check();
    delay(500);
    if (wifi__verbonden) {
      Serial.println("app poging 1");
      verzonden = verzend_loop(bericht);
    }
    if (!verzonden) {
      // AT reset
      Serial.println("AT_reset nodig");
      sendData("AT+RST\r\n", 5000, false);
      delay(5000);
      wifi_check();
      delay(500);
      if (wifi__verbonden) {
        Serial.println("app poging 2");
        verzonden = verzend_loop(bericht);
      }
      if (!verzonden) {
        // Standaard verbinden
        Serial.println("Helaas, reset alleen was niet genoeg");
        wifi_verbinden();
        delay(3500);
        wifi_check();
        delay(500);
        if (wifi__verbonden) {
          Serial.println("app poging 2");
          verzonden = verzend_loop(bericht);
        }
        if (!verzonden) {
          // Verbinden, maar dan perse met de gevonden netwerken
          wifi_verbinden(false);
          delay(3500);
          wifi_check();
          delay(500);
          if (wifi__verbonden) {
            Serial.println("app poging 3");
            verzonden = verzend_loop(bericht);
          }
          if (!verzonden) {
            // Eerst opnieuw naar netwerken zoeken en dan verbinden met de gevonden netwerken
            wifi_beschikbare_netwerken_vernieuwen();
            wifi_verbinden(false);
            delay(3500);
            wifi_check();
            delay(500);
            if (wifi__verbonden) {
              Serial.println("app poging 4");
              verzonden = verzend_loop(bericht);
            }
          }
        }
        
      }
    } else {
      Serial.print("wifi nog verbonden met ");
      Serial.println(wifi__ssid);
    }
  }

  if (wifi_connectie_modus != 1) {
    wifi_verbinding_verbreken();
  }

  return verzonden;

}

bool verzend_loop(String bericht) {
  
  int gelukt = 0;
  int mislukt = 0;

  if (wifi__verbonden) {
    String response;
    
    bericht.replace("+", " ");
    beeld_melding("Poging to zenden", bericht, "Bericht versturen");
    bericht.replace(" ", "+");

    for (int i = 0; i < app_cnt; i++) {
      response = "";
      if (app_app[i] == "signal") {
        response = signal_app(bericht, app_tel[i], app_key[i]);
      } else if (app_app[i] == "whatsapp") {
        response = whatsapp_app(bericht, app_tel[i], app_key[i]);
      }
      
      
      if (((response.indexOf("Apikey Correct.") > 0) && (response.indexOf("Script ended before Timeout.") > 0)) || ((response.indexOf("Text to send") > 0) && (response.indexOf("You will receive it") > 0))) {
        beeld_melding("Bericht verzonden", app_tel[i], "Bericht versturen");
        Serial.println("Versturen bericht");
        laatste_app = millis();
        gelukt++;
        if (i == app_cnt-1) {
          delay(3000);
        }
      } else if ((response.indexOf("Apikey Correct.") > 0) && (response.indexOf("ERROR: There is currently a limit") > 0)) {
        beeld_melding("Te veel berichten", "bericht is niet verzonden, maar de code is juist");
        Serial.println("Bericht niet verzonden vanwegen de gestelde limiet");
        mislukt++;
      } else {
        beeld_melding("Verzenden mislukt", app_tel[i], "Bericht versturen");
        Serial.println("Bericht niet verzonden");
        mislukt++;
        if (i == app_cnt-1) {
          delay(5000);
        } else {
          delay(1000);
        }
      }
    }
  } else {
    beeld_melding("Verzenden mislukt", "Geen verbinding met wifi");
    Serial.println("Verzenden mislukt, geen verbinding met wifi :(");
    mislukt++;
  }
  delay(1500);

  if (mislukt == 0) {
    return true;
  } else {
    return false;
  }
}



String signal_app(String bericht, String TELNR, String CM_KEY) {
  /* Deze functie verstuurt het bericht via signal en geeft de response van de api terug */

  String host = "api.callmebot.com";
  String url = "/signal/send.php?phone=" + TELNR + "&apikey=" + CM_KEY + "&text=";
  sendData("AT+CIPSTART=\"TCP\",\"" + host + "\",80\r\n\r\n", 8000, DEBUG);        
  String commando = "GET " + url + bericht + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
  Serial.println(commando.length());
  sendData("AT+CIPSEND=" + String(commando.length()) + "\r\n", 1500, DEBUG);
  Serial.println(host + commando);
  String response = sendData(commando + "\r\n", 4000, DEBUG);

  return response;
      
}

String whatsapp_app(String bericht, String TELNR, String CM_KEY) {
  /* Deze functie verstuurt het bericht via signal en geeft de response van de api terug */

  String host = "api.callmebot.com";
  String url = "/whatsapp.php?phone=" + TELNR + "&apikey=" + CM_KEY + "&text=";
  sendData("AT+CIPSTART=\"TCP\",\"" + host + "\",80\r\n\r\n", 8000, DEBUG);        
  String commando = "GET " + url + bericht + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
  Serial.println(commando.length());
  sendData("AT+CIPSEND=" + String(commando.length()) + "\r\n", 1500, DEBUG);
  Serial.println(host + commando);
  String response = sendData(commando + "\r\n", 4000, DEBUG);

  return response;
      
}