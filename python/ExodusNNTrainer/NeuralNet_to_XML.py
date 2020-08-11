import torch
import xml.etree.ElementTree as ET

def get_XML_format(model):
    neural_net = ET.Element('NeuralNet')

    params = model.parameters()
    modules = model.named_modules()

    for idx,m in enumerate(modules):
        if idx > 0:
            layer = ET.SubElement(neural_net,'LAYER')
            layer.set("ID",str(idx-1) )
            if (type(m[1]) == torch.nn.Linear ):
                weight_tensor = next(params)
                bias_tensor = next(params)
                m,n = weight_tensor.shape
                weight = [str(i) for i in weight_tensor.data.flatten().tolist()]
                bias   = [str(i) for i in bias_tensor.data.flatten().tolist()]

                type_node   = ET.SubElement(layer,"TYPE")
                type_node.text = "LINEAR"

                weight_node = ET.SubElement(layer,"WEIGHTS")
                weight_node.text = " ".join(weight)

                bias_node   = ET.SubElement(layer,"BIAS")
                bias_node.text = " ".join(bias)

                m_node = ET.SubElement(layer,"M")
                m_node.text = str(m)

                n_node = ET.SubElement(layer,"N")
                n_node.text = str(n)

            elif type(m[1]) == torch.nn.LogSigmoid:
                type_node   = ET.SubElement(layer,"TYPE")
                type_node.text = "LOGSIGMOID"
                continue
            elif type(m[1]) == torch.nn.Sigmoid:
                type_node   = ET.SubElement(layer,"TYPE")
                type_node.text = "SIGMOID"
                continue
            elif type(m[1]) == torch.nn.Tanh:
                type_node   = ET.SubElement(layer,"TYPE")
                type_node.text = "TANH"
                continue


    nn_string = ET.tostring(neural_net)
    return nn_string

##Tests code if run directly from shell
if __name__ == '__main__':
    D_in = 3
    H = 20
    D_out = 4

    model = torch.nn.Sequential(
        torch.nn.Linear(D_in,H),
        torch.nn.Tanh(),
        torch.nn.Linear(H,H),
        torch.nn.Tanh(),
        torch.nn.Linear(H,D_out),
        )

    loss_fn = torch.nn.MSELoss(reduction='sum')

    XML_str = (get_XML_format(model))
    with open('NN_struct.XML','wb+') as file:
        file.write(XML_str)
