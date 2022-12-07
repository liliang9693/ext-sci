

//% color="#1296db" iconWidth=50 iconHeight=40
namespace gravitysci {
    //% block="[OBJ] init while success i2c address[ADDR]" blockType="command"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    //% ADDR.shadow="dropdown" ADDR.options="ADDR" 
    export function init(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let addr=parameter.ADDR.code;
        console.log(obj)
        obj=obj.replace(/"/g,"") //去引号
        console.log(obj)
        Generator.addInclude("rp2040lib", `#include "DFRobot_RP2040_SCI.h"`);
        Generator.addObject(`rp2040obj${obj}`, `DFRobot_RP2040_SCI_IIC`, `${obj}(/*addr=*/${addr}, &Wire);`);
        Generator.addCode([`while(${obj}.begin() != 0){delay(1000);}`, Generator.ORDER_UNARY_POSTFIX]);
 
    }
   
    
    //% block="[OBJ] get [KEY] [VOU]" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1"
    //% KEY.shadow="string" KEY.defl="Light" 
    //% VOU.shadow="dropdown" VOU.options="VOU" 
    export function getKeyValueOrUnit(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let key=parameter.KEY.code;
        let vou=parameter.VOU.code;
        obj=obj.replace(/"/g,"") //去引号
        Generator.addCode(`${obj}.get${vou}(${key})`);
 
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
        obj=obj.replace(/"/g,"") //去引号

        Generator.addCode(`${obj}.get${vou}(${obj}.${port},${key})`);
 
    }

    //% block="[OBJ] get firmware version" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    export function getVersion(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        obj=obj.replace(/"/g,"") //去引号

        Generator.addCode(`${obj}.getVersionDescription(${obj}.getVersion())`);
        
    }

    //% block="[OBJ] get time stamp" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1" 
    export function getTimeStamp(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        obj=obj.replace(/"/g,"") //去引号

        Generator.addCode(`${obj}.getTimeStamp()`);
        
    }

 
}
