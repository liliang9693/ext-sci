

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
        Generator.addInclude("rp2040lib", `#include "DFRobot_RP2040_SUAB.h"`);
        Generator.addObject(`rp2040obj`, `DFRobot_RP2040_SUAB_IIC`, `${obj}(/*addr=*/${addr}, &Wire);`);
        Generator.addCode([`while(${obj}.begin() != 0){delay(1000);}`, Generator.ORDER_UNARY_POSTFIX]);
 
    }
   
    
    //% block="[OBJ] get [KEY] value" blockType="reporter"
    //% OBJ.shadow="normal" OBJ.defl="SCI1"
    //% KEY.shadow="string" KEY.defl="Light" 
    export function getValue(parameter: any, block: any) {
        let obj=parameter.OBJ.code;
        let key=parameter.KEY.code;
        obj=obj.replace(/"/g,"") //去引号
        Generator.addCode(`${obj}.getValue(${key})`);
 
    }


 
}
