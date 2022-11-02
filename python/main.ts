
//% color="#1296db" iconWidth=50 iconHeight=40
namespace gravitysci {

    //% block="[OBJ] init while success i2c address[ADDR]" blockType="command"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% ADDR.shadow="dropdown" ADDR.options="ADDR" 
    export function init(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let addr=parameter.ADDR.code;
        console.log(obj)
        Generator.addImport("from dfrobot_rp2040_suab import *");
        Generator.addImport("import time");
        Generator.addCode(`${obj} = DFRobot_SUAB_IIC(addr=${addr})
while ${obj}.begin() != 0:
    print("Initialization Sensor Universal Adapter Board failed.")
    time.sleep(1)
print("Initialization Sensor Universal Adapter Board done.")`);
 
    }
   
    
    //% block="[OBJ] get [KEY] value" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% KEY.shadow="string" KEY.defl="Light" 
    export function getValue(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let key=parameter.KEY.code;
        console.log(obj)

        Generator.addCode(`${obj}.get_value0(${key})`);
 
    }
   

}