    How to run the web server program:
    Steps:
    1) Run weather.py
    2) In the terminal click on the url provided
    3) In the url http://127.0.0.1:5000/city
       Change (city) to any city in the world.
    4) Hit Enter Key
    5) The Weather information is now displayed
        
        Notes:

        The program displays current weather. However, it may not be user friendly because the user has to manipulate the source url to find the weather information of a specified city. To go around this, we can use a text box in the .html file and the user can type any city they desire, and have code to fetch the user input file. This is a very difficult task. I have added ideas of code in the html file that adds the writeable text box below, but retrieving the input is very difficult. 

        Another problem is updating automatically, the user has to type in the city everytime they want the weather of that city. We need a continuous and updated display. So regard this as version 1 of the web server application. 

        Furthermore, there is a static folder with a .css file. This is used to manipulate the appearance of the web server. This is placed in the backburner of the sprint as it is not important. 
        
        Ideas for textbox:
        Code is inside <body>

        <h1>Weather</h1>
     
        Weather in: <input type="text" id="place" name="place"><br><br>

        <button onclick="myFunction()">Try it</button>
        
        <p id="demo"></p>

        <script>
            function myFunction() {
                var my_place = document.getElementById("place").value;
                document.getElementById("demo").innerHTML = my_place;
            }

        </script>