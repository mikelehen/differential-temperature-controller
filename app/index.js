// Require static content
require.context('./', true, /\.(html|webmanifest|te?xt)$/);
require.context('./', true, /\.(png|jpe?g|gif|svg|ico)$/);

// Require styles
require('normalize.css');
require('./css/styles.scss');

const firebase = require('firebase/app');
require('firebase/database');
const Chart = require('chart.js');

firebase.initializeApp({
  apiKey: 'AIzaSyBkwSFy0FpmrIOpJtYs0NiG7V9ktv1Nksc',
  authDomain: 'solarpool-4f1cd.firebaseapp.com',
  databaseURL: 'https://solarpool-4f1cd.firebaseio.com',
  storageBucket: 'solarpool-4f1cd.appspot.com',
  messagingSenderId: '538978285523',
});

const log = [];

const tempChart = new Chart(document.getElementById('tempLog').getContext('2d'), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Pool °F',
      yAxisID: 'y-axis-1',
      backgroundColor: 'rgba(54,162,235,0.2)',
      borderColor: 'rgba(54,162,235,1)',
      pointRadius: 0,
      pointHitRadius: 4,
      data: [],
    }, {
      label: 'Collector °F',
      yAxisID: 'y-axis-1',
      backgroundColor: 'rgba(255,99,132,0.2)',
      borderColor: 'rgba(255,99,132,1)',
      pointRadius: 0,
      pointHitRadius: 4,
      data: [],
    }, {
      label: 'Collector Active',
      yAxisID: 'y-axis-2',
      backgroundColor: 'rgba(255,99,132,0.2)',
      borderColor: 'transparent',
      pointRadius: 0,
      pointHitRadius: 4,
      data: [],
    }],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    stacked: false,
    scales: {
      xAxes: [{
        scaleType: 'linear',
        gridLines: {
          offsetGridLines: false,
        },
      }],
      yAxes: [{
        scaleType: 'linear',
        display: true,
        position: 'left',
        horizontal: false,
        id: 'y-axis-1',

        gridLines: {
          show: true,
          color: 'rgba(0, 0, 0, 0.05)',
          lineWidth: 1,
          drawOnChartArea: true,
          drawTicks: true,
          zeroLineWidth: 1,
          zeroLineColor: 'rgba(0,0,0,0.25)',
        },

        beginAtZero: true,
        integersOnly: false,
        override: null,

        labels: {
          show: true,
          template: '<%=value%>',
          fontSize: 12,
          fontStyle: 'normal',
          fontColor: '#666',
          fontFamily: 'Helvetica Neue',
        },
      }, {
        scaleType: 'linear',
        display: false,
        position: 'right',
        horizontal: false,
        id: 'y-axis-2',

        gridLines: {
          show: true,
          color: 'rgba(0, 0, 0, 0.05)',
          lineWidth: 1,
          drawOnChartArea: true,
          drawTicks: true,
          zeroLineWidth: 1,
          zeroLineColor: 'rgba(0,0,0,0.25)',
        },

        beginAtZero: true,
        integersOnly: false,
        override: null,

        labels: {
          show: true,
          template: '<%=value%>',
          fontSize: 12,
          fontStyle: 'normal',
          fontColor: '#666',
          fontFamily: 'Helvetica Neue',
        },
      }],
    },
  },
});

let config = {
  pollingMilliseconds: 5000,
};

firebase.database().ref('config').on('value', value => {
  config = value.val();
});

let updatePending = false;

function updateDataSet() {
  updatePending = false;
  const ordered = log.sort((left, right) => left.time - right.time);

  tempChart.data.labels = ordered.map(
      (sample) => new Date(sample.time * 1000).toISOString().slice(0, 16));

  const extractTemps = (channel) => {
    tempChart.data.datasets[channel].data = ordered.map((sample) => {
      const adc = sample[channel];
      const rs = config.seriesResistor;
      const r = rs / ((1023.0 / adc) - 1.0);

      const k = 273.15;
      const r0 = config.resistanceAt0;
      const t0 = config.temperatureAt0 + k;
      const b = config.bCoefficient;

      const c = (1.0 / ((Math.log(r / r0) / b) + (1.0 / t0))) - k;
      return (c * 1.8 + 32.0).toFixed(2);
    });
  };

  extractTemps(0);
  extractTemps(1);

  tempChart.data.datasets[2].data = ordered.map(
    (sample) => (sample.active ? 1 : 0));

  tempChart.update();
}

const update = (child) => {
  const index = child.key;
  const value = child.val();
  log[index] = value;
  if (!updatePending) {
    setTimeout(updateDataSet, 3000);
    updatePending = true;
  }
};

const logRef = firebase.database().ref('log');
logRef.on('child_added', update);
logRef.on('child_changed', update);
