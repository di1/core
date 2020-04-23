#!/bin/python
import bs4 as BeautifulSoup
from bs4 import SoupStrainer
import re
import urllib.request
import pandas as pd
import requests
import time

headers = {'User-agent': 'Mozilla/5.0'}
mySymbols = {}

symbols_txt = open('../symbols.csv')
lines = symbols_txt.readlines()
symbols = [l.split(',')[0] for l in lines]

# looping through all my symbols
for s in symbols:
    try:
        vals = {}
        #getting the symbol "profile" from Yahoo finance.
        # The url for the stock appears on this page.
        url = ("https://finance.yahoo.com/quote/{}/profile?p={}".format(s,s))
        webpage = requests.get(url, headers=headers)
        soup = BeautifulSoup.BeautifulSoup(webpage.content, features='lxml')

        # the title has the company name but has additional information in the format of
        # (SSS) profile and ....
        # where SSS is the symbol. We remove this extra title to get the company name.
        title = soup.find("title")
        tmp = title.get_text()
        rxTitle = re.compile(r'\(.*$')
        coName = rxTitle.sub("", tmp)

        # looping through all the links in the document.
        # The company web site is the the one that doesn't have yahoo in the reference,
        # and has a blank title.
        for link in soup.find_all('a', href=True):
            try:
                if link['target'] and "" == link['title']:
                    m = re.search('yahoo', link['href'], flags=re.IGNORECASE)
                    if None == m:

                        url = link['href']
                        webpage = requests.get(url, headers=headers)
                        soup = BeautifulSoup.BeautifulSoup(webpage.content, features='lxml')

                        vals = {"company":coName, "url":link['href']}
                        mySymbols[s] = vals
            except:
                pass
        try:
            logo_url = 'https://logo.clearbit.com/' + mySymbols[s]['url']
            filename = './icons/' + s + '.png'
            r = requests.get(logo_url, timeout=0.5)

            if r.status_code == 200:
                with open(filename, 'wb') as f:
                    f.write(r.content)
        except:
            print('error:', s)
            pass
    except:
        print('error:', s)
    time.sleep(1)
