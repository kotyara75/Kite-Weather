var pointsOfTheCompass = [
     {point: "North", abbreviation: "N", minAzimuth: 354.38, midAzimuth: 0, maxAzimuth: 5.62},
     {point: "North by east", abbreviation: "NbE", minAzimuth: 5.63, midAzimuth: 11.25, maxAzimuth: 16.87},
     {point: "North-northeast", abbreviation: "NNE", minAzimuth: 16.88, midAzimuth: 22.5, maxAzimuth: 28.12},
     {point: "Northeast by north", abbreviation: "NEbN", minAzimuth: 28.13, midAzimuth: 33.75, maxAzimuth: 39.37},
     {point: "Northeast", abbreviation: "NE", minAzimuth: 39.38, midAzimuth: 45, maxAzimuth: 50.62},
     {point: "Northeast by east", abbreviation: "NEbE", minAzimuth: 50.63, midAzimuth: 56.25, maxAzimuth: 61.87},
     {point: "East-northeast", abbreviation: "ENE", minAzimuth: 61.88, midAzimuth: 67.5, maxAzimuth: 73.12},
     {point: "East by north", abbreviation: "EbN", minAzimuth: 73.13, midAzimuth: 78.75, maxAzimuth: 84.37},
     {point: "East", abbreviation: "E", minAzimuth: 84.38, midAzimuth: 90, maxAzimuth: 95.62},
     {point: "East by south", abbreviation: "EbS", minAzimuth: 95.63, midAzimuth: 101.25, maxAzimuth: 106.87},
     {point: "East-southeast", abbreviation: "ESE", minAzimuth: 106.88, midAzimuth: 112.5, maxAzimuth: 118.12},
     {point: "Southeast by east", abbreviation: "SEbE", minAzimuth: 118.13, midAzimuth: 123.75, maxAzimuth: 129.37},
     {point: "Southeast", abbreviation: "SE", minAzimuth: 129.38, midAzimuth: 135, maxAzimuth: 140.62},
     {point: "Southeast by south", abbreviation: "SEbS", minAzimuth: 140.63, midAzimuth: 146.25, maxAzimuth: 151.87},
     {point: "South-southeast", abbreviation: "SSE", minAzimuth: 151.88, midAzimuth: 157.5, maxAzimuth: 163.12},
     {point: "South by east", abbreviation: "SbE", minAzimuth: 163.13, midAzimuth: 168.75, maxAzimuth: 174.37},
     {point: "South", abbreviation: "S", minAzimuth: 174.38, midAzimuth: 180, maxAzimuth: 185.62},
     {point: "South by west", abbreviation: "SbW", minAzimuth: 185.63, midAzimuth: 191.25, maxAzimuth: 196.87},
     {point: "South-southwest", abbreviation: "SSW", minAzimuth: 196.88, midAzimuth: 202.5, maxAzimuth: 208.12},
     {point: "Southwest by south", abbreviation: "SWbS", minAzimuth: 208.13, midAzimuth: 213.75, maxAzimuth: 219.37},
     {point: "Southwest", abbreviation: "SW", minAzimuth: 219.38, midAzimuth: 225, maxAzimuth: 230.62},
     {point: "Southwest by west", abbreviation: "SWbW", minAzimuth: 230.63, midAzimuth: 236.25, maxAzimuth: 241.87},
     {point: "West-southwest", abbreviation: "WSW", minAzimuth: 241.88, midAzimuth: 247.5, maxAzimuth: 253.12},
     {point: "West by south", abbreviation: "WbS", minAzimuth: 253.13, midAzimuth: 258.75, maxAzimuth: 264.37},
     {point: "West", abbreviation: "W", minAzimuth: 264.38, midAzimuth: 270, maxAzimuth: 275.62},
     {point: "West by north", abbreviation: "WbN", minAzimuth: 275.63, midAzimuth: 281.25, maxAzimuth: 286.87},
     {point: "West-northwest", abbreviation: "WNW", minAzimuth: 286.88, midAzimuth: 292.5, maxAzimuth: 298.12},
     {point: "Northwest by west", abbreviation: "NWbW", minAzimuth: 298.13, midAzimuth: 303.75, maxAzimuth: 309.37},
     {point: "Northwest", abbreviation: "NW", minAzimuth: 309.38, midAzimuth: 315, maxAzimuth: 320.62},
     {point: "Northwest by north", abbreviation: "NWbN", minAzimuth: 320.63, midAzimuth: 326.25, maxAzimuth: 331.87},
     {point: "North-northwest", abbreviation: "NNW", minAzimuth: 331.88, midAzimuth: 337.5, maxAzimuth: 343.12},
     {point: "North by west", abbreviation: "NbW", minAzimuth: 343.13, midAzimuth: 348.75, maxAzimuth: 354.37}
    ];

function azimuthToAbbreviation(deg) {
    var withinAzimuth = function (item) {
        var found = (deg >= item.minAzimuth) && (deg <= item.maxAzimuth);
        return found;
    };

    var points = pointsOfTheCompass.filter(withinAzimuth);
    
    return points[0].abbreviation;
}
module.exports.azimuthToAbbreviation = azimuthToAbbreviation;


function abbreviationToAzimuth(abbr) {
    var isPoint = function (item) {
        return (abbr == item.abbreviation);
    };
    
    var point = pointsOfTheCompass.filter(isPoint);
    
    return point.midAzimuth;
}
module.exports.abbreviationToAzimuth = abbreviationToAzimuth;

