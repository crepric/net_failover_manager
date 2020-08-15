"""
This file is part of Net Failover Manager.

Net Failover Manager is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Net Failover Manager is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Net Failover Manager.  If not, see <https://www.gnu.org/licenses/>.
"""

import os
from flask import Flask, render_template, jsonify, request
from absl import app
from absl import flags
from absl import logging
from rules_python.python.runfiles import runfiles
from google.protobuf.json_format import MessageToJson

import grpc
from src.proto import net_failover_manager_service_pb2
from src.proto import net_failover_manager_service_pb2_grpc

r = runfiles.Create()
runfiles_dir = r.EnvVars()['RUNFILES_DIR']
TEMPLATE_FOLDER = 'net_failover_manager/web_ui/templates/'
STATIC_FOLDER = 'net_failover_manager/web_ui/resources/'
flaskapp = Flask(__name__,
                 template_folder=os.path.join(runfiles_dir, TEMPLATE_FOLDER),
                 static_folder=os.path.join(runfiles_dir, STATIC_FOLDER),
                 static_url_path="")


FLAGS = flags.FLAGS

channel = grpc.insecure_channel('localhost:50051')
stub = net_failover_manager_service_pb2_grpc.NetworkConfigStub(channel)


@flaskapp.route('/')
def serve():
    return render_template('index.html')


@flaskapp.route('/get_default_gw', methods=['GET'])
def getDefaultGateway():
    grpc_request = net_failover_manager_service_pb2.DefaultGwRequest()
    grpc_response = stub.GetDefaultGw(grpc_request)
    return jsonify({'default_gw': grpc_response.default_gw_interface})


@flaskapp.route('/get_interface_status', methods=['GET'])
def getInterfaceStatuses():
    grpc_request = net_failover_manager_service_pb2.IfStatusRequest()
    grpc_response = stub.GetIfStatus(grpc_request)
    return MessageToJson(grpc_response)


@flaskapp.route('/set_default_gw', methods=['GET'])
def setDefaultGateway():
    new_interface = request.args.get('interface')
    grpc_request = net_failover_manager_service_pb2.ForceNewGatewayRequest()
    grpc_request.if_name = new_interface
    grpc_response = stub.ForceNewGateway(grpc_request)
    return jsonify({'result': 'OK'})


def main(argv):
    flaskapp.run(host="0.0.0.0", port=8000)


if __name__ == "__main__":
    app.run(main)
