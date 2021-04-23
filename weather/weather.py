from flask import Flask, render_template
import requests
from datetime import datetime

# Reference weather fetch code: https://github.com/KeithGalli/GUI
# api.openweathermap.org/data/2.5/forecast?q={city name},{country code}
# a4aa5e3d83ffefaba8c00284de6ef7c3
	
def get_weather(city):

        weather_key = 'a4aa5e3d83ffefaba8c00284de6ef7c3'
        url_weather = 'https://api.openweathermap.org/data/2.5/weather'
        params = {'APPID': weather_key, 'q': city, 'units': 'imperial'}
        response = requests.get(url_weather, params=params)
        weather = response.json()
        return weather
  
app = Flask(__name__)

@app.route('/<place>')

def get_city(place):

     weather_information =  get_weather(place)
     environment=weather_information['weather'][0]['description']
     temp = round((weather_information['main']['temp'] - 32) * (5/9), 1)

     utc_time = datetime.utcnow()
     utc_string = utc_time.strftime('%Y%m%d %H%M%S ')

     return render_template('weather.html', the_city=place, the_environment=environment, the_temp=temp, the_time=utc_string)
      
if __name__ == '__main__':
        app.debug = True
        app.run(host="0.0.0.0")