# Arduino Projects

Embedded systems and Arduino projects from my Computer Engineering coursework at Queen's University. These projects explore sensor integration, actuator control, real-time data, and adaptive decision making.

| Project | What it does | Technologies |
| --- | --- | --- |
| [Rock-Paper-Scissors Strategy](Arduino_RPS_Strategy/) | Uses a 3×3 transition table to predict an opponent's next move and choose a counter. Built for the ELEC 290 competition, where our team placed first. | Arduino, C++, probabilistic strategy |
| [Anxiety Support Prototype and Dashboard](Anxiety-Dashboard/) | Combines GSR and motion sensing with a vibration motor, LED breathing pattern, and browser visualization. The sensor thresholds are experimental, not clinical measurements of anxiety. | Arduino, MPU6050, GSR, JavaScript, WebSocket |
| [Water Filtration Prototype](Water-Filtration/) | Coordinates liquid-level sensing, two turbidity sensor readings, pumps, and a servo in a team-built water treatment prototype. Turbidity values are monitored but do not currently control dosage. | Arduino Mega, C++, sensors, motors |

Each project folder contains its source and a project-specific README with hardware and setup details.

## Running the projects

- **Rock-Paper-Scissors:** Open the sketch in the Arduino IDE and consult its [strategy explanation](Arduino_RPS_Strategy/README.md). Confirm the board and hardware setup before uploading.
- **Anxiety support prototype:** See the [project README](Anxiety-Dashboard/README.md) for connections and setup. Run the included Node.js serial-to-WebSocket bridge alongside the Arduino sketch to stream readings to the browser.
- **Water filtration:** See the [project README](Water-Filtration/README.md) for the pin map, library requirements, and prototype limitations.

These are educational prototypes. The anxiety and water treatment projects are not validated medical or drinking-water systems.

## About me

I'm Aram Azadian, a Computer Engineering student at Queen's University interested in embedded systems and software development. See my [GitHub profile](https://github.com/Aram-az) or [LinkedIn](https://www.linkedin.com/in/aram-azadian/) for more work.
