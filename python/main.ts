
//% color="#1296db" iconWidth=50 iconHeight=40
namespace gravitysci {

    //% block="[OBJ] init while success i2c address[ADDR]" blockType="command"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% ADDR.shadow="dropdown" ADDR.options="ADDR" 
    export function init(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let addr=parameter.ADDR.code;
        console.log(obj)
        Generator.addImport("from dfrobot_rp2040_sci import *");
        Generator.addImport("import time");
        Generator.addCode(`${obj} = DFRobot_RP2040_SCI_IIC(addr=${addr})
while ${obj}.begin() != 0:
    print("Initialization Sensor Universal Adapter Board failed.")
    time.sleep(1)
print("Initialization Sensor Universal Adapter Board done.")`);
 
    }
       //% block="---"
       export function noteSep2() {

       }
    
    //% block="[OBJ] get [KEY] [VOU]" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% KEY.shadow="string" KEY.defl="Light" 
    //% VOU.shadow="dropdown" VOU.options="VOU" 
    export function getKeyValueOrUnit(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let key=parameter.KEY.code;
        let vou=parameter.VOU.code;

        Generator.addCode(`${obj}.get_${vou}0(${key})`);

        
 
    }


    //% block="[OBJ] get port[PORT] [KEY] [VOU]" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% PORT.shadow="dropdown" PORT.options="PORT" 
    //% KEY.shadow="string" KEY.defl="Light" 
    //% VOU.shadow="dropdown" VOU.options="VOU" 
    export function getPortKeyValue(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let port=parameter.PORT.code;
        let key=parameter.KEY.code;
        let vou=parameter.VOU.code;

        Generator.addCode(`${obj}.get_${vou}1(${obj}.${port},${key})`);

        
    }

    
    //% block="[OBJ] get firmware version" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    export function getVersion(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        obj=obj.replace(/"/g,"") //去引号

        Generator.addCode(`${obj}.get_version_description(${obj}.get_version())`);
        
    }

    //% block="[OBJ] get time stamp" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    export function getTimeStamp(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        obj=obj.replace(/"/g,"") //去引号

        Generator.addCode(`${obj}.get_timestamp()`);
        
    }

    //% block="---"
    export function noteSep1() {

    }
    //% block="[OBJ] set record [SWITCH]" blockType="command"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% SWITCH.shadow="dropdown" SWITCH.options="SWITCH" 
    export function setRecord(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let sw=parameter.SWITCH.code;

        obj=obj.replace(/"/g,"") //去引号

        if(sw=="ON"){
            Generator.addCode(`${obj}.enable_record()`);
        }else if(sw=="OFF"){
            Generator.addCode(`${obj}.disable_record()`);
        }
        
        
    }

  
   

}
