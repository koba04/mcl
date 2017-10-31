const she = require("./she-node");
const range = 2048;
const tryNum = 100;
she().init(range, tryNum, () => {
  console.log("ok");
});
